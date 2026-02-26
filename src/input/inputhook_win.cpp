#include "inputhook.h"

#ifdef Q_OS_WIN

#include <QAtomicInt>
#include <QDebug>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <windows.h>

class InputHookWin::Impl {
  public:
    HHOOK keyboardHook = nullptr;
    HHOOK mouseHook = nullptr;
    QThread *thread = nullptr;
    std::atomic<bool> running{false};
    std::atomic<bool> shouldStop{false};
    InputHookWin *q = nullptr;
    DWORD threadId = 0; // stored when thread starts

    POINT lastMousePos = {0, 0};
    bool hasLastMousePos = false;

    static const UINT WM_STOP_HOOK = WM_USER + 100;

    static LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            auto *pKbStruct = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
            if (pKbStruct) {
                InputHookWin *hook = instance();
                if (hook) {
                    bool pressed = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
                    DWORD vkCode = pKbStruct->vkCode;
                    if (vkCode == VK_RETURN && (pKbStruct->flags & LLKHF_EXTENDED)) {
                        vkCode = 1025;
                    }
                    if (pressed)
                        emit hook->keyPressed(vkCode, 0);
                    else
                        emit hook->keyReleased(vkCode, 0);
                }
            }
        }
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    static LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            auto *pMouseStruct = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
            if (pMouseStruct) {
                InputHookWin *hook = instance();
                if (!hook) return CallNextHookEx(nullptr, nCode, wParam, lParam);

                switch (wParam) {
                case WM_LBUTTONDOWN:
                    emit hook->mousePressed(0);
                    break;
                case WM_LBUTTONUP:
                    emit hook->mouseReleased(0);
                    break;
                case WM_RBUTTONDOWN:
                    emit hook->mousePressed(1);
                    break;
                case WM_RBUTTONUP:
                    emit hook->mouseReleased(1);
                    break;
                case WM_MBUTTONDOWN:
                    emit hook->mousePressed(2);
                    break;
                case WM_MBUTTONUP:
                    emit hook->mouseReleased(2);
                    break;
                case WM_XBUTTONDOWN: {
                    int button = (HIWORD(pMouseStruct->mouseData) == XBUTTON1) ? 3 : 4;
                    emit hook->mousePressed(button);
                    break;
                }
                case WM_XBUTTONUP: {
                    int button = (HIWORD(pMouseStruct->mouseData) == XBUTTON1) ? 3 : 4;
                    emit hook->mouseReleased(button);
                    break;
                }
                case WM_MOUSEWHEEL: {
                    int delta = GET_WHEEL_DELTA_WPARAM(pMouseStruct->mouseData);
                    int direction = (delta > 0) ? 0 : 1;
                    emit hook->mouseScrolled(direction, static_cast<double>(delta));
                    break;
                }
                case WM_MOUSEHWHEEL: {
                    int delta = GET_WHEEL_DELTA_WPARAM(pMouseStruct->mouseData);
                    int direction = (delta > 0) ? 3 : 2;
                    emit hook->mouseScrolled(direction, static_cast<double>(delta));
                    break;
                }
                case WM_MOUSEMOVE:
                case WM_NCMOUSEMOVE: {
                    if (hook->impl->hasLastMousePos) {
                        double dx = static_cast<double>(pMouseStruct->pt.x - hook->impl->lastMousePos.x);
                        double dy = static_cast<double>(pMouseStruct->pt.y - hook->impl->lastMousePos.y);
                        emit hook->mouseMoved(dx, dy);
                    }
                    hook->impl->lastMousePos = pMouseStruct->pt;
                    hook->impl->hasLastMousePos = true;
                    break;
                }
                default:
                    break;
                }
            }
        }
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    static InputHookWin *&instance() {
        static thread_local InputHookWin *inst = nullptr;
        return inst;
    }

    static void threadMain(InputHookWin *hook) {
        instance() = hook;
        Impl *impl = hook->impl.get();

        // Store the thread ID
        impl->threadId = GetCurrentThreadId();

        impl->keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, GetModuleHandle(nullptr), 0);
        impl->mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, GetModuleHandle(nullptr), 0);

        if (!impl->keyboardHook || !impl->mouseHook) {
            qWarning() << "Failed to set Windows hooks";
            if (impl->keyboardHook) UnhookWindowsHookEx(impl->keyboardHook);
            if (impl->mouseHook) UnhookWindowsHookEx(impl->mouseHook);
            impl->keyboardHook = nullptr;
            impl->mouseHook = nullptr;
            return;
        }

        impl->running = true;
        MSG msg;
        while (!impl->shouldStop && GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_STOP_HOOK)
                break;
        }

        if (impl->keyboardHook) {
            UnhookWindowsHookEx(impl->keyboardHook);
            impl->keyboardHook = nullptr;
        }
        if (impl->mouseHook) {
            UnhookWindowsHookEx(impl->mouseHook);
            impl->mouseHook = nullptr;
        }
        impl->running = false;
        instance() = nullptr;
    }
};

InputHookWin::InputHookWin(QObject *parent)
    : InputHook(parent), impl(std::make_unique<Impl>()) {
    impl->q = this;
}

InputHookWin::~InputHookWin() {
    stop();
}

bool InputHookWin::start() {
    qDebug() << "InputWinHook::start()";

    if (impl->running) return true;
    impl->shouldStop = false;
    impl->hasLastMousePos = false;
    impl->threadId = 0; // reset

    impl->thread = QThread::create([this]() { Impl::threadMain(this); });
    connect(impl->thread, &QThread::finished, impl->thread, &QObject::deleteLater);
    impl->thread->start();
    return true;
}

void InputHookWin::stop() {
    if (!impl->running) return;
    impl->shouldStop = true;
    if (impl->thread && impl->thread->isRunning()) {
        if (impl->threadId != 0) {
            PostThreadMessage(impl->threadId, Impl::WM_STOP_HOOK, 0, 0);
        }
        impl->thread->quit();
        impl->thread->wait();
    }
    impl->thread = nullptr;
    impl->running = false;
}

bool InputHookWin::isRunning() const {
    return impl->running;
}

#endif // Q_OS_WIN