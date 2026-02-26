#include "loaders/keyboardloader.h"
#include "mainwindow.h"
#include "models/settings.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
    // Check for --debug flag
    bool debug = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug = true;
            break;
        }
    }
#ifdef Q_OS_WIN
    if (debug) {
        if (AllocConsole()) {
            FILE *fDummy;
            freopen_s(&fDummy, "CONOUT$", "w", stdout);
            freopen_s(&fDummy, "CONOUT$", "w", stderr);
        } else {
            // If allocation fails, show a message box
            MessageBoxA(NULL, "Failed to allocate console", "NohBoard Debug", MB_OK | MB_ICONERROR);
        }
    }
#endif

    QApplication app(argc, argv);
    app.setApplicationName("NohBoard Qt");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("NohBoard");

    AppSettings settings;
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/NohBoard.json";
    QFile file(settingsPath);
    bool settingsExist = file.exists();

    if (settingsExist && file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            settings.mouseSensitivity = obj.value("MouseSensitivity").toInt(50);
            settings.updateInterval = obj.value("UpdateInterval").toInt(33);
            settings.scrollHold = obj.value("ScrollHold").toInt(50);
            settings.mouseFromCenter = obj.value("MouseFromCenter").toBool(false);
            settings.pressHold = obj.value("PressHold").toInt(0);
            settings.windowTitle = obj.value("WindowTitle").toString("NohBoard Qt");
            settings.capitalization = obj.value("Capitalization").toInt(0);
            settings.followShiftForCapsInsensitive = obj.value("FollowShiftForCapsInsensitive").toBool(true);
            settings.followShiftForCapsSensitive = obj.value("FollowShiftForCapsSensitive").toBool(false);
            settings.trapKeyboard = obj.value("TrapKeyboard").toBool(false);
            settings.trapMouse = obj.value("TrapMouse").toBool(false);
            settings.trapToggleKeyCode = obj.value("TrapToggleKeyCode").toInt(145);
            settings.updateTextPosition = obj.value("UpdateTextPosition").toBool(true);
            settings.loadedCategory = obj.value("LoadedCategory").toString();
            settings.loadedKeyboard = obj.value("LoadedKeyboard").toString();
            settings.loadedStyle = obj.value("LoadedStyle").toString();
            settings.loadedGlobalStyle = obj.value("LoadedGlobalStyle").toBool(false);
            settings.windowPosition = QPoint(obj.value("X").toInt(0), obj.value("Y").toInt(0));
        }
    } else {
        // First run – set defaults
        settings.loadedCategory = "Normal";
        settings.loadedKeyboard = "us_qwerty";
        settings.loadedStyle = "light";
        settings.windowPosition = QPoint(100, 100); // will be centered by MainWindow

        // Save defaults
        QJsonObject obj;
        obj["MouseSensitivity"] = settings.mouseSensitivity;
        obj["UpdateInterval"] = settings.updateInterval;
        obj["ScrollHold"] = settings.scrollHold;
        obj["MouseFromCenter"] = settings.mouseFromCenter;
        obj["PressHold"] = settings.pressHold;
        obj["WindowTitle"] = settings.windowTitle;
        obj["Capitalization"] = settings.capitalization;
        obj["FollowShiftForCapsInsensitive"] = settings.followShiftForCapsInsensitive;
        obj["FollowShiftForCapsSensitive"] = settings.followShiftForCapsSensitive;
        obj["TrapKeyboard"] = settings.trapKeyboard;
        obj["TrapMouse"] = settings.trapMouse;
        obj["TrapToggleKeyCode"] = settings.trapToggleKeyCode;
        obj["UpdateTextPosition"] = settings.updateTextPosition;
        obj["LoadedCategory"] = settings.loadedCategory;
        obj["LoadedKeyboard"] = settings.loadedKeyboard;
        obj["LoadedStyle"] = settings.loadedStyle;
        obj["LoadedGlobalStyle"] = settings.loadedGlobalStyle;
        obj["X"] = settings.windowPosition.x();
        obj["Y"] = settings.windowPosition.y();

        QDir().mkpath(QFileInfo(settingsPath).path());
        QFile outFile(settingsPath);
        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(QJsonDocument(obj).toJson());
            outFile.close();
        }
    }
    KeyboardLoader::getDefaultKeyboardsPath();

    qDebug() << "Starting NohBoard Qt";
    qDebug() << "Platform:" << QGuiApplication::platformName();

    MainWindow window(settings);
    window.show();

    return app.exec();
}