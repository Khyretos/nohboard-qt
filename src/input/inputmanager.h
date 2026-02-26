#pragma once
#include "inputhook.h"
#include <QMutex>
#include <QObject>
#include <QPoint>
#include <QSet>
#include <memory>

class InputManager : public QObject {
    Q_OBJECT
  public:
    explicit InputManager(QObject *parent = nullptr);
    ~InputManager();

    bool start();
    void stop();
    bool isRunning() const;

    bool isKeyPressed(int keyCode) const;
    bool isMouseButtonPressed(int button) const;
    QPointF mouseSpeed() const;

    bool isCapsLockOn() const { return m_capsLockOn; }

  signals:
    void keyStateChanged(int keyCode, bool pressed);
    void mouseButtonChanged(int button, bool pressed);
    void mouseScrolled(int direction);
    void mouseMoved(float dx, float dy);

  private:
    std::unique_ptr<InputHook> m_hook;
    mutable QMutex m_mutex;
    QSet<int> m_pressedKeys;
    QSet<int> m_pressedMouseButtons;
    float m_mouseSpeedX = 0.f;
    float m_mouseSpeedY = 0.f;
    bool m_capsLockOn = false;
};
