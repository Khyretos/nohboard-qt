#include "inputhook.h"

#ifdef Q_OS_MACOS

#include <ApplicationServices/ApplicationServices.h>
#include <QAtomicInt>
#include <QDebug>
#include <QMap>
#include <QMutex>
#include <QThread>

// -----------------------------------------------------------------------------
// macOS Virtual Key Code to Windows VK mapping (US layout)
// -----------------------------------------------------------------------------
static int macKeyCodeToWindowsVK(CGKeyCode code) {
    static const QMap<CGKeyCode, int> map = {
        {0x00, 0x41}, // A
        {0x01, 0x53}, // S
        {0x02, 0x44}, // D
        {0x03, 0x46}, // F
        {0x04, 0x48}, // H
        {0x05, 0x47}, // G
        {0x06, 0x5A}, // Z
        {0x07, 0x58}, // X
        {0x08, 0x43}, // C
        {0x09, 0x56}, // V
        {0x0A, 0x42}, // B
        {0x0B, 0x51}, // Q
        {0x0C, 0x57}, // W
        {0x0D, 0x45}, // E
        {0x0E, 0x52}, // R
        {0x0F, 0x59}, // Y
        {0x10, 0x54}, // T
        {0x11, 0x31}, // 1
        {0x12, 0x32}, // 2
        {0x13, 0x33}, // 3
        {0x14, 0x34}, // 4
        {0x15, 0x36}, // 6
        {0x16, 0x35}, // 5
        {0x17, 0xBD}, // =
        {0x18, 0x39}, // 9
        {0x19, 0x37}, // 7
        {0x1A, 0xBD}, // -
        {0x1B, 0x38}, // 8
        {0x1C, 0x30}, // 0
        {0x1D, 0xDD}, // ]
        {0x1E, 0x4F}, // O
        {0x1F, 0x55}, // U
        {0x20, 0xDB}, // [
        {0x21, 0x49}, // I
        {0x22, 0x50}, // P
        {0x23, 0x0D}, // Return
        {0x24, 0x4C}, // L
        {0x25, 0x4A}, // J
        {0x26, 0xDE}, // '
        {0x27, 0x4B}, // K
        {0x28, 0xBA}, // ;
        {0x29, 0xDC}, // \
        { 0x2A, 0xBC }, // ,
        {0x2B, 0xBF}, // /
        {0x2C, 0x4E}, // N
        {0x2D, 0x4D}, // M
        {0x2E, 0xBE}, // .
        {0x2F, 0x09}, // Tab
        {0x30, 0x20}, // Space
        {0x31, 0xC0}, // `
        {0x32, 0x08}, // Backspace
        {0x33, 0x1B}, // Escape
        {0x7A, 0x70}, // F1
        {0x78, 0x71}, // F2
        {0x63, 0x72}, // F3
        {0x76, 0x73}, // F4
        {0x60, 0x74}, // F5
        {0x61, 0x75}, // F6
        {0x62, 0x76}, // F7
        {0x64, 0x77}, // F8
        {0x65, 0x78}, // F9
        {0x6D, 0x79}, // F10
        {0x67, 0x7A}, // F11
        {0x6F, 0x7B}, // F12
        {0x38, 0xA0}, // Left Shift
        {0x3C, 0xA0}, // Right Shift
        {0x3B, 0xA2}, // Left Control
        {0x3E, 0xA2}, // Right Control
        {0x3A, 0xA4}, // Left Alt
        {0x3D, 0xA4}, // Right Alt
        {0x37, 0x5B}, // Left Command
        {0x36, 0x5C}, // Right Command
        {0x7B, 0x25}, // Left Arrow
        {0x7C, 0x26}, // Right Arrow
        {0x7D, 0x28}, // Down Arrow
        {0x7E, 0x27}, // Up Arrow
        {0x73, 0x24}, // Home
        {0x77, 0x23}, // End
        {0x74, 0x21}, // Page Up
        {0x79, 0x22}, // Page Down
        {0x75, 0x2E}, // Delete
        {0x72, 0x2D}, // Insert
        // Keypad
        {0x52, 0x60}, // KP 0
        {0x53, 0x61}, // KP 1
        {0x54, 0x62}, // KP 2
        {0x55, 0x63}, // KP 3
        {0x56, 0x64}, // KP 4
        {0x57, 0x65}, // KP 5
        {0x58, 0x66}, // KP 6
        {0x59, 0x67}, // KP 7
        {0x5B, 0x68}, // KP 8
        {0x5C, 0x69}, // KP 9
        {0x45, 0x6E}, // KP .
        {0x4E, 0x6B}, // KP +
        {0x4B, 0x6D}, // KP -
        {0x43, 0x6A}, // KP *
        {0x4C, 0x6F}, // KP /
        {0x4A, 0x0D}, // KP Enter
        {0x47, 0x90}, // Clear
    };
    return map.value(code, code);
}

// -----------------------------------------------------------------------------
// macOS Input Hook Implementation
// -----------------------------------------------------------------------------
class InputHookMac::Impl {
  public:
    CFMachPortRef eventTap = nullptr;
    CFRunLoopSourceRef runLoopSource = nullptr;
    CFRunLoopRef runLoop = nullptr;
    QThread *thread = nullptr;
    std::atomic<bool> running{false};
    std::atomic<bool> shouldStop{false};
    InputHookMac *q = nullptr;

    static CGEventRef eventTapCallback(CGEventTapProxy proxy, CGEventType type,
                                       CGEventRef event, void *userInfo) {
        Q_UNUSED(proxy);
        auto *impl = static_cast<Impl *>(userInfo);
        if (!impl || !impl->q) return event;

        switch (type) {
        case kCGEventKeyDown:
        case kCGEventKeyUp: {
            CGKeyCode keyCode = static_cast<CGKeyCode>(
                CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
            int vk = macKeyCodeToWindowsVK(keyCode);
            if (type == kCGEventKeyDown)
                emit impl->q->keyPressed(vk, 0);
            else
                emit impl->q->keyReleased(vk, 0);
            break;
        }
        case kCGEventFlagsChanged:
            // Modifier keys only – we can synthesize later if needed
            break;
        case kCGEventLeftMouseDown:
            emit impl->q->mousePressed(0);
            break;
        case kCGEventLeftMouseUp:
            emit impl->q->mouseReleased(0);
            break;
        case kCGEventRightMouseDown:
            emit impl->q->mousePressed(1);
            break;
        case kCGEventRightMouseUp:
            emit impl->q->mouseReleased(1);
            break;
        case kCGEventOtherMouseDown: {
            int button = static_cast<int>(CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber));
            emit impl->q->mousePressed(button);
            break;
        }
        case kCGEventOtherMouseUp: {
            int button = static_cast<int>(CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber));
            emit impl->q->mouseReleased(button);
            break;
        }
        case kCGEventMouseMoved:
        case kCGEventLeftMouseDragged:
        case kCGEventRightMouseDragged:
        case kCGEventOtherMouseDragged: {
            double dx = CGEventGetDoubleValueField(event, kCGMouseEventDeltaX);
            double dy = CGEventGetDoubleValueField(event, kCGMouseEventDeltaY);
            emit impl->q->mouseMoved(dx, dy);
            break;
        }
        case kCGEventScrollWheel: {
            int64_t scrollY = CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1);
            int64_t scrollX = CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis2);
            if (scrollY != 0) {
                int direction = (scrollY > 0) ? 1 : 0; // down = 1, up = 0
                emit impl->q->mouseScrolled(direction, static_cast<double>(scrollY));
            }
            if (scrollX != 0) {
                int direction = (scrollX > 0) ? 3 : 2; // right = 3, left = 2
                emit impl->q->mouseScrolled(direction, static_cast<double>(scrollX));
            }
            break;
        }
        case kCGEventTapDisabledByTimeout:
        case kCGEventTapDisabledByUserInput:
            qWarning() << "macOS event tap disabled, attempting to restart";
            impl->q->stop();
            impl->q->start();
            break;
        default:
            break;
        }
        return event;
    }

    static void threadMain(Impl *impl) {
        impl->runLoop = CFRunLoopGetCurrent();
        CFRunLoopAddSource(impl->runLoop, impl->runLoopSource, kCFRunLoopDefaultMode);
        CFRunLoopRun();
        CFRunLoopRemoveSource(impl->runLoop, impl->runLoopSource, kCFRunLoopDefaultMode);
    }
};

InputHookMac::InputHookMac(QObject *parent)
    : InputHook(parent), impl(std::make_unique<Impl>()) {
    impl->q = this;
}

InputHookMac::~InputHookMac() {
    stop();
    // unique_ptr automatically deletes impl, no manual delete
}

bool InputHookMac::start() {
    if (impl->running) return true;

    CGEventMask mask = (1 << kCGEventKeyDown) | (1 << kCGEventKeyUp) |
                       (1 << kCGEventFlagsChanged) |
                       (1 << kCGEventLeftMouseDown) | (1 << kCGEventLeftMouseUp) |
                       (1 << kCGEventRightMouseDown) | (1 << kCGEventRightMouseUp) |
                       (1 << kCGEventOtherMouseDown) | (1 << kCGEventOtherMouseUp) |
                       (1 << kCGEventMouseMoved) |
                       (1 << kCGEventLeftMouseDragged) |
                       (1 << kCGEventRightMouseDragged) |
                       (1 << kCGEventOtherMouseDragged) |
                       (1 << kCGEventScrollWheel);

    impl->eventTap = CGEventTapCreate(kCGSessionEventTap,
                                      kCGHeadInsertEventTap,
                                      kCGEventTapOptionListenOnly,
                                      mask,
                                      Impl::eventTapCallback,
                                      impl.get()); // pass raw pointer

    if (!impl->eventTap) {
        qWarning() << "Failed to create macOS event tap. Check accessibility permissions.";
        return false;
    }

    impl->runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, impl->eventTap, 0);
    if (!impl->runLoopSource) {
        qWarning() << "Failed to create run loop source for event tap";
        CFRelease(impl->eventTap);
        impl->eventTap = nullptr;
        return false;
    }

    impl->running = true;
    impl->shouldStop = false;

    impl->thread = QThread::create([this]() { Impl::threadMain(impl.get()); });
    connect(impl->thread, &QThread::finished, impl->thread, &QObject::deleteLater);
    impl->thread->start();

    return true;
}

void InputHookMac::stop() {
    if (!impl->running) return;
    impl->shouldStop = true;

    if (impl->runLoop) {
        CFRunLoopStop(impl->runLoop);
        impl->runLoop = nullptr;
    }

    if (impl->thread && impl->thread->isRunning()) {
        impl->thread->quit();
        impl->thread->wait();
    }
    impl->thread = nullptr;

    if (impl->runLoopSource) {
        CFRelease(impl->runLoopSource);
        impl->runLoopSource = nullptr;
    }
    if (impl->eventTap) {
        CFRelease(impl->eventTap);
        impl->eventTap = nullptr;
    }

    impl->running = false;
}

bool InputHookMac::isRunning() const {
    return impl->running;
}

#endif // Q_OS_MACOS