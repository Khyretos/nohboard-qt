#include "inputhook.h"
#include <QDebug>
#include <QGuiApplication>

InputHook *InputHook::create(QObject *parent) {
#if defined(Q_OS_WIN)
    qDebug() << "Creating Windows input hook";
    return new InputHookWin(parent);
#elif defined(Q_OS_MACOS)
    qDebug() << "Creating macOS input hook";
    return new InputHookMac(parent);
#elif defined(USE_WAYLAND) || defined(USE_X11)
    // Linux: detect platform at runtime
    QString platform = QGuiApplication::platformName();
#ifdef USE_WAYLAND
    if (platform == "wayland") {
        qDebug() << "Creating Wayland input hook";
        return new InputHookWayland(parent);
    }
#endif
#ifdef USE_X11
    if (platform == "xcb" || platform.contains("x11", Qt::CaseInsensitive)) {
        qDebug() << "Creating X11 input hook";
        return new InputHookX11(parent);
    }
#endif
    // Fallback (should not happen if at least one is enabled)
#ifdef USE_WAYLAND
    qDebug() << "Unknown platform, defaulting to Wayland";
    return new InputHookWayland(parent);
#elif defined(USE_X11)
    qDebug() << "Unknown platform, defaulting to X11";
    return new InputHookX11(parent);
#else
#error "No input hook implementation for this platform!"
#endif
#else
#error "No input hook implementation for this platform!"
#endif
}