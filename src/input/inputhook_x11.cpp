// Include X11 headers first
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

// Undefine all X11 macros that conflict with Qt
#undef Status      // conflicts with QTextStream::Status
#undef Bool        // conflicts with QMetaType::Bool
#undef CursorShape // conflicts with Qt::CursorShape
#undef True        // conflicts with Qt's bool values
#undef False
#undef None // conflicts with Qt::None (e.g., in window flags)

// Now include our headers and Qt headers
#include "inputhook.h"
#include <QDebug>
#include <QThread>
#include <mutex>

class InputHookX11::Impl
{
public:
    Display *display = nullptr;
    Display *recordDisplay = nullptr;
    XRecordContext context = 0;
    std::mutex mutex;
    bool running = false;

    static void eventCallback(XPointer ptr, XRecordInterceptData *data);
};

void InputHookX11::Impl::eventCallback(XPointer ptr, XRecordInterceptData *data)
{
    if (!data)
        return;

    InputHookX11 *hook = reinterpret_cast<InputHookX11 *>(ptr);

    if (data->category == XRecordFromServer)
    {
        const unsigned char *eventData = data->data;
        int eventType = eventData[0] & 0x7F;

        if (eventType == KeyPress || eventType == KeyRelease)
        {
            unsigned char detail = eventData[1];

            Display *dpy = hook->impl->display;
            if (!dpy)
            {
                XRecordFreeData(data);
                return;
            }

            KeySym ks = NoSymbol;
            int keysyms_per_keycode;
            KeySym *keysyms = XGetKeyboardMapping(dpy, detail, 1, &keysyms_per_keycode);
            if (keysyms)
            {
                ks = keysyms[0];
                XFree(keysyms);
            }

            if (ks != NoSymbol)
            {
                if (eventType == KeyPress)
                    emit hook->keyPressed(detail, ks);
                else
                    emit hook->keyReleased(detail, ks);
            }
        }
        else if (eventType == ButtonPress || eventType == ButtonRelease)
        {
            unsigned char button = eventData[1];

            if (eventType == ButtonPress)
                emit hook->mousePressed(button);
            else
                emit hook->mouseReleased(button);
        }
    }

    XRecordFreeData(data);
}

InputHookX11::InputHookX11(QObject *parent)
    : InputHook(parent), impl(new Impl())
{
}

InputHookX11::~InputHookX11()
{
    stop();
    delete impl;
}

bool InputHookX11::start()
{
    std::lock_guard<std::mutex> lock(impl->mutex);

    if (impl->running)
        return true;

    impl->display = XOpenDisplay(nullptr);
    impl->recordDisplay = XOpenDisplay(nullptr);

    if (!impl->display || !impl->recordDisplay)
    {
        qWarning() << "Failed to open X11 display";
        stop();
        return false;
    }

    int major, minor;
    if (!XRecordQueryVersion(impl->recordDisplay, &major, &minor))
    {
        qWarning() << "RECORD extension not available";
        stop();
        return false;
    }

    XRecordRange *range = XRecordAllocRange();
    if (!range)
    {
        stop();
        return false;
    }

    range->device_events.first = KeyPress;
    range->device_events.last = MotionNotify;

    XRecordClientSpec clientSpec = XRecordAllClients;

    impl->context = XRecordCreateContext(impl->recordDisplay, 0, &clientSpec, 1, &range, 1);
    XFree(range);

    if (!impl->context)
    {
        qWarning() << "Failed to create record context";
        stop();
        return false;
    }

    impl->running = true;

    QThread *thread = QThread::create([this]()
                                      { XRecordEnableContext(impl->recordDisplay, impl->context,
                                                             Impl::eventCallback, reinterpret_cast<XPointer>(this)); });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();

    return true;
}

void InputHookX11::stop()
{
    std::lock_guard<std::mutex> lock(impl->mutex);

    if (!impl->running)
        return;

    impl->running = false;

    if (impl->context && impl->recordDisplay)
    {
        XRecordDisableContext(impl->recordDisplay, impl->context);
        XRecordFreeContext(impl->recordDisplay, impl->context);
        impl->context = 0;
    }

    if (impl->recordDisplay)
    {
        XCloseDisplay(impl->recordDisplay);
        impl->recordDisplay = nullptr;
    }

    if (impl->display)
    {
        XCloseDisplay(impl->display);
        impl->display = nullptr;
    }
}

bool InputHookX11::isRunning() const
{
    return impl->running;
}