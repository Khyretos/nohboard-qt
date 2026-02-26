#pragma once
#include <QPoint>
#include <QString>

struct AppSettings {
    int mouseSensitivity = 50;
    int updateInterval = 33;
    bool trapKeyboard = false;
    bool trapMouse = false;
    int trapToggleKeyCode = 145;
    bool followShiftForCapsInsensitive = true;
    bool followShiftForCapsSensitive = false;
    bool mouseFromCenter = false;
    int pressHold = 0;
    int scrollHold = 50;
    QString windowTitle = "NohBoard Qt";
    QPoint windowPosition = QPoint(0, 0);
    QString loadedCategory;
    QString loadedKeyboard;
    QString loadedStyle;
    bool loadedGlobalStyle = false;
    int capitalization = 0;
    bool alwaysOnTop = true;
    bool verboseLogging = false;
    bool updateTextPosition = true; // add this if missing
};