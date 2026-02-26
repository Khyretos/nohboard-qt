#include "inputmanager.h"
#include "../logger.h"

#ifdef Q_OS_LINUX
#include <linux/input-event-codes.h>
#endif

static const QString COMP = "InputManager";

static int mapMouseButton(unsigned int btn) {
    switch (btn) {
    case 0x110:
        return 0; // left
    case 0x111:
        return 1; // right
    case 0x112:
        return 2; // middle
    case 0x113:
        return 3; // side (X1)
    case 0x114:
        return 4; // extra (X2)
    default:
        return -1; // ignore unknown
    }
}

#ifdef Q_OS_LINUX
// Convert Linux evdev keycode to Windows virtual key code (VK)
static int evdevToWindowsVK(int evdev) {
    // Based on linux/input-event-codes.h and WinUser.h
    switch (evdev) {
    // Letters
    case KEY_A:
        return 0x41; // A
    case KEY_B:
        return 0x42; // B
    case KEY_C:
        return 0x43; // C
    case KEY_D:
        return 0x44; // D
    case KEY_E:
        return 0x45; // E
    case KEY_F:
        return 0x46; // F
    case KEY_G:
        return 0x47; // G
    case KEY_H:
        return 0x48; // H
    case KEY_I:
        return 0x49; // I
    case KEY_J:
        return 0x4A; // J
    case KEY_K:
        return 0x4B; // K
    case KEY_L:
        return 0x4C; // L
    case KEY_M:
        return 0x4D; // M
    case KEY_N:
        return 0x4E; // N
    case KEY_O:
        return 0x4F; // O
    case KEY_P:
        return 0x50; // P
    case KEY_Q:
        return 0x51; // Q
    case KEY_R:
        return 0x52; // R
    case KEY_S:
        return 0x53; // S
    case KEY_T:
        return 0x54; // T
    case KEY_U:
        return 0x55; // U
    case KEY_V:
        return 0x56; // V
    case KEY_W:
        return 0x57; // W
    case KEY_X:
        return 0x58; // X
    case KEY_Y:
        return 0x59; // Y
    case KEY_Z:
        return 0x5A; // Z

    // Numbers (top row)
    case KEY_1:
        return 0x31; // 1
    case KEY_2:
        return 0x32; // 2
    case KEY_3:
        return 0x33; // 3
    case KEY_4:
        return 0x34; // 4
    case KEY_5:
        return 0x35; // 5
    case KEY_6:
        return 0x36; // 6
    case KEY_7:
        return 0x37; // 7
    case KEY_8:
        return 0x38; // 8
    case KEY_9:
        return 0x39; // 9
    case KEY_0:
        return 0x30; // 0

    // Function keys
    case KEY_F1:
        return 0x70; // F1
    case KEY_F2:
        return 0x71; // F2
    case KEY_F3:
        return 0x72; // F3
    case KEY_F4:
        return 0x73; // F4
    case KEY_F5:
        return 0x74; // F5
    case KEY_F6:
        return 0x75; // F6
    case KEY_F7:
        return 0x76; // F7
    case KEY_F8:
        return 0x77; // F8
    case KEY_F9:
        return 0x78; // F9
    case KEY_F10:
        return 0x79; // F10
    case KEY_F11:
        return 0x7A; // F11
    case KEY_F12:
        return 0x7B; // F12

    // Keypad
    case KEY_KP0:
        return 0x60; // Numpad 0
    case KEY_KP1:
        return 0x61; // Numpad 1
    case KEY_KP2:
        return 0x62; // Numpad 2
    case KEY_KP3:
        return 0x63; // Numpad 3
    case KEY_KP4:
        return 0x64; // Numpad 4
    case KEY_KP5:
        return 0x65; // Numpad 5
    case KEY_KP6:
        return 0x66; // Numpad 6
    case KEY_KP7:
        return 0x67; // Numpad 7
    case KEY_KP8:
        return 0x68; // Numpad 8
    case KEY_KP9:
        return 0x69; // Numpad 9
    case KEY_KPDOT:
        return 0x6E; // Numpad .
    case KEY_KPPLUS:
        return 0x6B; // Numpad +
    case KEY_KPMINUS:
        return 0x6D; // Numpad -
    case KEY_KPASTERISK:
        return 0x6A; // Numpad *
    case KEY_KPSLASH:
        return 0x6F; // Numpad /
    case KEY_KPENTER:
        return 0x0D; // Numpad Enter (same as regular Enter? but extended)
    case KEY_NUMLOCK:
        return 0x90; // Num Lock

    // Modifiers
    case KEY_LEFTCTRL:
        return 0xA2; // Left Ctrl  (VK_LCONTROL)
    case KEY_RIGHTCTRL:
        return 0xA3; // Right Ctrl (VK_RCONTROL)
    case KEY_LEFTSHIFT:
        return 0xA0; // Left Shift (VK_LSHIFT)
    case KEY_RIGHTSHIFT:
        return 0xA1; // Right Shift (VK_RSHIFT)
    case KEY_LEFTALT:
        return 0xA4; // Left Alt   (VK_LMENU)
    case KEY_RIGHTALT:
        return 0xA5; // Right Alt  (VK_RMENU)
    case KEY_LEFTMETA:
        return 0x5B; // Left Win   (VK_LWIN)
    case KEY_RIGHTMETA:
        return 0x5C; // Right Win  (VK_RWIN)
    case KEY_MENU:
        return 0x5D; // Menu       (VK_APPS)

    // Editing keys
    case KEY_BACKSPACE:
        return 0x08; // Backspace
    case KEY_TAB:
        return 0x09; // Tab
    case KEY_ENTER:
        return 0x0D; // Enter
    case KEY_ESC:
        return 0x1B; // Escape
    case KEY_INSERT:
        return 0x2D; // Insert
    case KEY_DELETE:
        return 0x2E; // Delete
    case KEY_HOME:
        return 0x24; // Home
    case KEY_END:
        return 0x23; // End
    case KEY_PAGEUP:
        return 0x21; // Page Up
    case KEY_PAGEDOWN:
        return 0x22; // Page Down

    // Cursor keys
    case KEY_UP:
        return 0x26; // Up
    case KEY_DOWN:
        return 0x28; // Down
    case KEY_LEFT:
        return 0x25; // Left
    case KEY_RIGHT:
        return 0x27; // Right

    // Punctuation (US layout)
    case KEY_MINUS:
        return 0xBD; // -
    case KEY_EQUAL:
        return 0xBB; // =
    case KEY_LEFTBRACE:
        return 0xDB; // [
    case KEY_RIGHTBRACE:
        return 0xDD; // ]
    case KEY_BACKSLASH:
        return 0xDC; // \/
    case KEY_SEMICOLON:
        return 0xBA; // ;
    case KEY_APOSTROPHE:
        return 0xDE; // '
    case KEY_GRAVE:
        return 0xC0; // `
    case KEY_COMMA:
        return 0xBC; // ,
    case KEY_DOT:
        return 0xBE; // .
    case KEY_SLASH:
        return 0xBF; // /

    // Caps Lock, Scroll Lock, etc.
    case KEY_CAPSLOCK:
        return 0x14; // Caps Lock
    case KEY_SCROLLLOCK:
        return 0x91; // Scroll Lock
    case KEY_PAUSE:
        return 0x13; // Pause/Break

    // Space
    case KEY_SPACE:
        return 0x20; // Space

    // Default: return evdev code as fallback (may still be wrong)
    default:
        return evdev;
    }
}
#endif

InputManager::InputManager(QObject *parent)
    : QObject(parent), m_hook(InputHook::create(this)) {
    LOG_INFO(COMP, "🎮 InputManager created");
    qDebug() << "InputManager constructor: hook created";

#ifdef Q_OS_LINUX
    // Linux: convert evdev keycodes to Windows VK
    connect(m_hook.get(), &InputHook::keyPressed, this, [this](unsigned int keycode, KeySym /*keysym*/) {
        int vk = evdevToWindowsVK(static_cast<int>(keycode));
        {
            QMutexLocker lock(&m_mutex);
            m_pressedKeys.insert(vk);
            LOG_DEBUG(COMP, QString("⌨️  Key %1 (evdev %2) ↓").arg(vk).arg(keycode));
        }
        emit keyStateChanged(vk, true);
    });

    connect(m_hook.get(), &InputHook::keyReleased, this, [this](unsigned int keycode, KeySym /*keysym*/) {
        int vk = evdevToWindowsVK(static_cast<int>(keycode));
        {
            QMutexLocker lock(&m_mutex);
            m_pressedKeys.remove(vk);
            LOG_DEBUG(COMP, QString("⌨️  Key %1 (evdev %2) ↑").arg(vk).arg(keycode));
        }
        emit keyStateChanged(vk, false);
    });
#else
    // Windows / macOS: keycode is already the correct VK
    connect(m_hook.get(), &InputHook::keyPressed, this, [this](unsigned int keycode, KeySym /*keysym*/) {
        {
            QMutexLocker lock(&m_mutex);
            m_pressedKeys.insert(static_cast<int>(keycode));
            LOG_DEBUG(COMP, QString("⌨️  Key %1 ↓").arg(keycode));
        }
        emit keyStateChanged(static_cast<int>(keycode), true);
    });

    connect(m_hook.get(), &InputHook::keyReleased, this, [this](unsigned int keycode, KeySym /*keysym*/) {
        {
            QMutexLocker lock(&m_mutex);
            m_pressedKeys.remove(static_cast<int>(keycode));
            LOG_DEBUG(COMP, QString("⌨️  Key %1 ↑").arg(keycode));
        }
        emit keyStateChanged(static_cast<int>(keycode), false);
    });
#endif

    // Mouse handling – platform‑specific
#ifdef Q_OS_LINUX
    // Linux: map raw X11/evdev button codes to 0‑4
    connect(m_hook.get(), &InputHook::mousePressed, this, [this](unsigned int button) {
        qDebug() << "InputManager::mousePressed raw:" << button;
        int mapped = mapMouseButton(button);
        if (mapped >= 0) {
            QMutexLocker lock(&m_mutex);
            m_pressedMouseButtons.insert(mapped);
            LOG_DEBUG(COMP, QString("🖱️  Mouse button %1 (raw %2) ↓").arg(mapped).arg(button));
            emit mouseButtonChanged(mapped, true);
        }
    });

    connect(m_hook.get(), &InputHook::mouseReleased, this, [this](unsigned int button) {
        qDebug() << "InputManager::mouseReleased raw:" << button;
        int mapped = mapMouseButton(button);
        if (mapped >= 0) {
            QMutexLocker lock(&m_mutex);
            m_pressedMouseButtons.remove(mapped);
            LOG_DEBUG(COMP, QString("🖱️  Mouse button %1 (raw %2) ↑").arg(mapped).arg(button));
            emit mouseButtonChanged(mapped, false);
        }
    });
#else
    // Windows/macOS: hook already emits normalized 0‑4
    connect(m_hook.get(), &InputHook::mousePressed, this, [this](unsigned int button) {
        qDebug() << "InputManager::mousePressed raw:" << button;
        int mapped = static_cast<int>(button);
        {
            QMutexLocker lock(&m_mutex);
            m_pressedMouseButtons.insert(mapped);
            LOG_DEBUG(COMP, QString("🖱️  Mouse button %1 ↓").arg(mapped));
        }
        emit mouseButtonChanged(mapped, true);
    });

    connect(m_hook.get(), &InputHook::mouseReleased, this, [this](unsigned int button) {
        qDebug() << "InputManager::mouseReleased raw:" << button;
        int mapped = static_cast<int>(button);
        {
            QMutexLocker lock(&m_mutex);
            m_pressedMouseButtons.remove(mapped);
            LOG_DEBUG(COMP, QString("🖱️  Mouse button %1 ↑").arg(mapped));
        }
        emit mouseButtonChanged(mapped, false);
    });
#endif

    // Mouse movement (same on all platforms)
    connect(m_hook.get(), &InputHook::mouseMoved, this, [this](double dx, double dy) {
        QMutexLocker lock(&m_mutex);
        m_mouseSpeedX = static_cast<float>(dx);
        m_mouseSpeedY = static_cast<float>(dy);
        // qDebug() << "InputManager::mouseMoved" << dx << dy;
        emit mouseMoved(static_cast<float>(dx), static_cast<float>(dy));
    });

    // Mouse scroll (same on all platforms)
    connect(m_hook.get(), &InputHook::mouseScrolled, this, [this](int direction, double value) {
        // Ignore tiny scroll events that might be noise from button presses
        if (qAbs(value) < 1.0) return;
        // qDebug() << "InputManager::mouseScrolled direction:" << direction << "value:" << value;
        emit mouseScrolled(direction);
    });
}

InputManager::~InputManager() {
    stop();
}

bool InputManager::start() {
    LOG_INFO(COMP, "▶️  Starting input hook...");
    qDebug() << "InputManager::start()";
    bool ok = m_hook->start();
    qDebug() << "InputManager::start() result:" << ok;

    if (ok)
        LOG_INFO(COMP, "✅ Input hook started successfully");
    else
        LOG_ERROR(COMP, "❌ Failed to start input hook");
    return ok;
}

void InputManager::stop() {
    if (m_hook->isRunning()) {
        LOG_INFO(COMP, "⏹️  Stopping input hook...");
        m_hook->stop();
    }
}

bool InputManager::isRunning() const {
    return m_hook->isRunning();
}

bool InputManager::isKeyPressed(int keyCode) const {
    QMutexLocker lock(&m_mutex);
    return m_pressedKeys.contains(keyCode);
}

bool InputManager::isMouseButtonPressed(int button) const {
    QMutexLocker lock(&m_mutex);
    return m_pressedMouseButtons.contains(button);
}

QPointF InputManager::mouseSpeed() const {
    QMutexLocker lock(&m_mutex);
    return QPointF(m_mouseSpeedX, m_mouseSpeedY);
}