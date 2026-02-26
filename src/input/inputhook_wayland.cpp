#include "inputhook.h"
#include <QSocketNotifier>
#include <QDebug>
#include <QGuiApplication>
// #include <qpa/qplatformnativeinterface.h>
#include <linux/input-event-codes.h>
#include <fcntl.h>
#include <unistd.h>
#include <libinput.h>
#include <libudev.h>

class InputHookWayland::Impl
{
public:
    struct udev *udev = nullptr;
    struct libinput *li = nullptr;
    QSocketNotifier *notifier = nullptr;
    int fd = -1;

    static int openRestricted(const char *path, int flags, void *user_data);
    static void closeRestricted(int fd, void *user_data);
};

int InputHookWayland::Impl::openRestricted(const char *path, int flags, void *user_data)
{
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}

void InputHookWayland::Impl::closeRestricted(int fd, void *user_data)
{
    close(fd);
}

InputHookWayland::InputHookWayland(QObject *parent)
    : InputHook(parent), impl(new Impl())
{
}

InputHookWayland::~InputHookWayland()
{
    stop();
    delete impl;
}

bool InputHookWayland::start()
{
    if (impl->li)
    {
        return true; // Already running
    }

    // Initialize udev
    impl->udev = udev_new();
    // After impl->udev = udev_new();
    if (impl->udev)
    {
        struct udev_enumerate *enumerate = udev_enumerate_new(impl->udev);
        udev_enumerate_add_match_subsystem(enumerate, "input");
        udev_enumerate_scan_devices(enumerate);
        struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
        struct udev_list_entry *entry;
        int count = 0;
        udev_list_entry_foreach(entry, devices)
        {
            const char *path = udev_list_entry_get_name(entry);
            qDebug() << "udev input device:" << path;
            count++;
        }
        qDebug() << "Found" << count << "input devices via udev";
        udev_enumerate_unref(enumerate);
    }
    // Setup libinput interface
    static const struct libinput_interface interface = {
        .open_restricted = Impl::openRestricted,
        .close_restricted = Impl::closeRestricted,
    };

    // Create libinput context
    impl->li = libinput_udev_create_context(&interface, nullptr, impl->udev);
    if (!impl->li)
    {
        qWarning() << "Failed to create libinput context";
        stop();
        return false;
    }

    // Assign seat
    if (libinput_udev_assign_seat(impl->li, "seat0") != 0)
    {
        qWarning() << "Failed to assign seat";
        stop();
        return false;
    }
    else
    {
        qDebug() << "Seat assigned successfully";
    }

    // Immediately dispatch to get initial device list
    libinput_dispatch(impl->li);

    // Process any initial events (like device added)
    struct libinput_event *ev;
    while ((ev = libinput_get_event(impl->li)))
    {
        qDebug() << "Initial event:" << libinput_event_get_type(ev);
        libinput_event_destroy(ev);
    }

    // Get file descriptor
    impl->fd = libinput_get_fd(impl->li);
    if (impl->fd < 0)
    {
        qWarning() << "Failed to get libinput fd";
        stop();
        return false;
    }

    // Setup socket notifier
    impl->notifier = new QSocketNotifier(impl->fd, QSocketNotifier::Read, this);
    connect(impl->notifier, &QSocketNotifier::activated, this, [this]()
            {
        // qDebug() << "Socket notifier activated";   // <-- add this

        libinput_dispatch(impl->li);
        
        struct libinput_event* event;
        while ((event = libinput_get_event(impl->li))) {
            handleEvent(event);
            libinput_event_destroy(event);
        } });

    return true;
}

void InputHookWayland::stop()
{
    if (impl->notifier)
    {
        delete impl->notifier;
        impl->notifier = nullptr;
    }

    if (impl->li)
    {
        libinput_unref(impl->li);
        impl->li = nullptr;
    }

    if (impl->udev)
    {
        udev_unref(impl->udev);
        impl->udev = nullptr;
    }

    impl->fd = -1;
}

bool InputHookWayland::isRunning() const
{
    return impl->li != nullptr;
}

void InputHookWayland::handleEvent(struct libinput_event *event)
{
    enum libinput_event_type type = libinput_event_get_type(event);
    // qDebug() << "InputHookWayland::handleEvent - type:" << type; // <-- add this

    switch (type)
    {
    case LIBINPUT_EVENT_KEYBOARD_KEY:
    {
        struct libinput_event_keyboard *kev = libinput_event_get_keyboard_event(event);
        uint32_t keycode = libinput_event_keyboard_get_key(kev);
        enum libinput_key_state state = libinput_event_keyboard_get_key_state(kev);
        // qDebug() << "  KEY event: keycode" << keycode << "state" << state; // <-- add this

        // Convert keycode to keysym
        KeySym keysym = keycode + 8; // temporary fallback

        if (state == LIBINPUT_KEY_STATE_PRESSED)
        {
            emit keyPressed(keycode, keysym);
        }
        else
        {
            emit keyReleased(keycode, keysym);
        }
        break;
    }

    case LIBINPUT_EVENT_POINTER_BUTTON:
    {
        struct libinput_event_pointer *pev = libinput_event_get_pointer_event(event);
        uint32_t button = libinput_event_pointer_get_button(pev);
        enum libinput_button_state state = libinput_event_pointer_get_button_state(pev);
        // qDebug() << "  MOUSE button event: button" << button << "state" << state; // <-- add this

        if (state == LIBINPUT_BUTTON_STATE_PRESSED)
        {
            emit mousePressed(button);
        }
        else
        {
            emit mouseReleased(button);
        }
        break;
    }

    case LIBINPUT_EVENT_POINTER_MOTION:
    {
        struct libinput_event_pointer *pev = libinput_event_get_pointer_event(event);
        double dx = libinput_event_pointer_get_dx(pev);
        double dy = libinput_event_pointer_get_dy(pev);
        emit mouseMoved(dx, dy);
        break;
    }

    case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
    {
        // qDebug() << "Scroll wheel event";
        struct libinput_event_pointer *pev = libinput_event_get_pointer_event(event);

        if (libinput_event_pointer_has_axis(pev, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
        {
            double value = libinput_event_pointer_get_scroll_value(pev, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
            int direction = (value > 0) ? 1 : 0;
            // qDebug() << "  Vertical scroll value:" << value << "direction:" << direction;
            emit mouseScrolled(direction, value);
        }

        if (libinput_event_pointer_has_axis(pev, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
        {
            double value = libinput_event_pointer_get_scroll_value(pev, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
            int direction = (value > 0) ? 3 : 2;
            // qDebug() << "  Horizontal scroll value:" << value << "direction:" << direction;
            emit mouseScrolled(direction, value);
        }
        break;
    }
    default:
        // qDebug() << "  other event type" << type; // <-- optional
        break;
    }
}