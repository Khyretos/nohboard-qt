#include "../logger.h"
#include "inputhook.h"

#ifdef USE_EVDEV
// Stub: evdev-based input hook (requires root or input group membership)
// For a real implementation, use libevdev to read from /dev/input/event*
static const QString COMP = "EvdevInputHook";

class EvdevInputHook : public InputHook {
  public:
    bool start() override {
        LOG_WARN(COMP, "⚠️  evdev input hook not fully implemented — keys won't be captured");
        m_running = true;
        return true;
    }
    void stop() override { m_running = false; }
    bool isRunning() const override { return m_running; }

  private:
    bool m_running = false;
};

InputHook *createInputHook() {
    return new EvdevInputHook();
}
#endif
