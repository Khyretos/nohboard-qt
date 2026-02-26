#ifndef INPUTHOOK_H
#define INPUTHOOK_H

#include <QObject>

// Forward declare KeySym to avoid X11 header conflicts with Qt
#ifdef USE_X11
typedef unsigned long KeySym;
#else
typedef unsigned long KeySym; // Define it anyway for Wayland builds
#endif

// Base class for input hooks
class InputHook : public QObject {
    Q_OBJECT

  public:
    explicit InputHook(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~InputHook() = default;

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;

    // Factory method to create appropriate hook for current platform
    static InputHook *create(QObject *parent = nullptr);

  signals:
    void keyPressed(unsigned int keycode, KeySym keysym);
    void keyReleased(unsigned int keycode, KeySym keysym);
    void mousePressed(unsigned int button);
    void mouseReleased(unsigned int button);
    void mouseMoved(double dx, double dy);
    void mouseScrolled(int direction, double value);
};

#ifdef USE_X11
// X11 implementation
class InputHookX11 : public InputHook {
    Q_OBJECT

  public:
    explicit InputHookX11(QObject *parent = nullptr);
    ~InputHookX11() override;

    bool start() override;
    void stop() override;
    bool isRunning() const override;

  private:
    class Impl;
    Impl *impl;
};
#endif

#ifdef USE_WAYLAND
// Wayland implementation
class InputHookWayland : public InputHook {
    Q_OBJECT

  public:
    explicit InputHookWayland(QObject *parent = nullptr);
    ~InputHookWayland() override;

    bool start() override;
    void stop() override;
    bool isRunning() const override;

  private:
    void handleEvent(struct libinput_event *event);

    class Impl;
    Impl *impl;
};
#endif

#ifdef Q_OS_WIN
class InputHookWin : public InputHook {
    Q_OBJECT
  public:
    explicit InputHookWin(QObject *parent = nullptr);
    ~InputHookWin() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;

  private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
#endif

#ifdef Q_OS_MACOS
class InputHookMac : public InputHook {
    Q_OBJECT
  public:
    explicit InputHookMac(QObject *parent = nullptr);
    ~InputHookMac() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;

  private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
#endif

#endif // INPUTHOOK_H