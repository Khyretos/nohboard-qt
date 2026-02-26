#include "mainwindow.h"
#include "dialogs/elementpropertiesdialog.h"
#include "dialogs/keyboardpropertiesdialog.h"
#include "dialogs/keyboardstyledialog.h"
#include "dialogs/keystyledialog.h"
#include "dialogs/loaddialog.h"
#include "dialogs/settingsdialog.h"
#include "keyboardwidget.h"
#include "loaders/keyboardloader.h"
#include "loaders/styleloader.h"
#include "models/settings.h"
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(const AppSettings &settings, QWidget *parent)
    : QMainWindow(parent),
      m_inputManager(new InputManager(this)),
      m_keyboardWidget(new KeyboardWidget(this, m_inputManager)),
      m_settings(settings) {
    qDebug() << "MainWindow constructor started";
    setWindowTitle(m_settings.windowTitle);
    resize(800, 600);
    statusBar()->hide();
    qDebug() << "UI setup...";
    setupUi();
    setupConnections();
    setWindowFlags(Qt::Window);

    // Force window to a visible position (center of primary screen)
    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - 800) / 2;
    int y = (screenGeometry.height() - 600) / 2;
    setGeometry(x, y, 800, 600);
    show();

    qDebug() << "Starting input manager...";
    if (!m_inputManager->start())
        qWarning() << "Failed to start input hook";
    else {
        qDebug() << "Input manager started";
    }

    QString keyboardsPath = KeyboardLoader::getDefaultKeyboardsPath();
    QString stylePath;
    QString defPath;

    if (!settings.loadedKeyboard.isEmpty()) {
        qDebug() << "Loading saved keyboard:" << settings.loadedCategory << "/" << settings.loadedKeyboard;
        defPath = keyboardsPath + "/" + settings.loadedCategory + "/" + settings.loadedKeyboard + "/keyboard.json";

        if (!settings.loadedStyle.isEmpty())
            stylePath = keyboardsPath + "/" + settings.loadedCategory + "/" + settings.loadedKeyboard + "/" + settings.loadedStyle + ".style";

    } else {
        defPath = keyboardsPath + "/Normal/us_intl_basicmouse/keyboard.json";
        stylePath = "/global/default.style";
    }
    m_keyboardWidget->loadKeyboard(defPath, stylePath);

    qDebug() << "MainWindow constructor finished";
}

MainWindow::~MainWindow() {
    if (m_inputManager) {
        m_inputManager->stop();
        delete m_inputManager; // if not using smart pointer
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    m_settings.windowPosition = pos();
    saveSettings();
    event->accept();
}

void MainWindow::setupUi() {
    // Setup menu bar
    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *settingsAction = fileMenu->addAction("&Settings");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);

    fileMenu->addSeparator();

    QAction *quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    // Setup central widget
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(m_keyboardWidget);
    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections() {
    // Connect input manager to keyboard widget
    connect(m_inputManager, &InputManager::keyStateChanged,
            m_keyboardWidget, &KeyboardWidget::onKeyStateChanged);
    connect(m_inputManager, &InputManager::mouseButtonChanged,
            m_keyboardWidget, &KeyboardWidget::onMouseButtonChanged);
    connect(m_inputManager, &InputManager::mouseMoved,
            m_keyboardWidget, &KeyboardWidget::onMouseMoved);
    // If you have scroll/move signals, connect them similarly

    // Connect keyboard widget signals
    connect(m_keyboardWidget, &KeyboardWidget::settingsRequested,
            this, &MainWindow::showSettings);
    connect(m_keyboardWidget, &KeyboardWidget::loadRequested,
            this, &MainWindow::showLoadDialog); // you'll add this signal later
    connect(m_inputManager, &InputManager::mouseScrolled,
            m_keyboardWidget, &KeyboardWidget::onMouseScrolled);
    connect(m_keyboardWidget, &KeyboardWidget::editRequested,
            this, &MainWindow::startEditing);
    connect(m_keyboardWidget, &KeyboardWidget::saveDefinitionRequested,
            this, &MainWindow::saveDefinition);
    connect(m_keyboardWidget, &KeyboardWidget::saveStyleRequested,
            this, &MainWindow::saveStyle);
    connect(m_keyboardWidget, &KeyboardWidget::editingChanged, this, [this](bool editing) {
        // Maybe update window title or status
    });
    connect(m_keyboardWidget, &KeyboardWidget::elementSelected, this, &MainWindow::onElementSelected);
    connect(m_keyboardWidget, &KeyboardWidget::showKeyboardProperties, this, &MainWindow::onShowKeyboardProperties);
    connect(m_keyboardWidget, &KeyboardWidget::showElementProperties, this, &MainWindow::onShowElementProperties);
    connect(m_keyboardWidget, &KeyboardWidget::showKeyboardStyle, this, &MainWindow::onShowKeyboardStyle);
    connect(m_keyboardWidget, &KeyboardWidget::showKeyStyle, this, &MainWindow::onShowKeyStyle);
}

void MainWindow::onShowElementProperties(int id) {
    auto elem = m_keyboardWidget->findElementById(id);
    if (!elem) return;
    ElementPropertiesDialog dlg(elem, m_keyboardWidget->width(), m_keyboardWidget->height(), this);
    if (dlg.exec() == QDialog::Accepted) {
        // Element is already modified via shared pointer, just force repaint
        m_keyboardWidget->update();
        // Optionally auto-save definition
        // m_keyboardWidget->saveDefinition(false);
    }
}
void MainWindow::onShowKeyboardStyle(const QString &path) {
    auto styleOpt = StyleLoader::load(path);
    if (!styleOpt) {
        QMessageBox::warning(this, "Error", "Could not load style file.");
        return;
    }
    KeyboardStyleDialog dlg(*styleOpt, this);
    if (dlg.exec() == QDialog::Accepted) {
        if (StyleLoader::save(dlg.style(), path)) {
            // Reload in keyboard widget
            m_keyboardWidget->loadKeyboard(m_keyboardWidget->getCurrentDefinitionPath(), path);
        } else {
            QMessageBox::warning(this, "Error", "Failed to save style.");
        }
    }
}

void MainWindow::onElementSelected(int id) {
    qDebug() << "Element selected:" << id;
    // Could highlight it
}

void MainWindow::onShowKeyboardProperties(int width, int height) {
    KeyboardPropertiesDialog dlg(width, height, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_keyboardWidget->setKeyboardSize(dlg.width(), dlg.height());
        // You'll need to add setKeyboardSize method that updates definition and resizes
    }
}

void MainWindow::onShowKeyStyle(int id, const QString &path) {
    auto elem = m_keyboardWidget->findElementById(id);
    if (!elem) return;

    auto styleOpt = StyleLoader::load(path);
    if (!styleOpt) return;

    bool hasOverride = styleOpt->keyElementStyles.contains(id);
    KeyStyle current = hasOverride ? styleOpt->keyElementStyles[id] : styleOpt->defaultKeyStyle;

    KeyStyleDialog dlg(current, hasOverride, this);
    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.overwriteDefault()) {
            styleOpt->keyElementStyles[id] = dlg.style();
        } else {
            styleOpt->keyElementStyles.remove(id);
        }
        if (StyleLoader::save(*styleOpt, path)) {
            m_keyboardWidget->loadKeyboard(m_keyboardWidget->getCurrentDefinitionPath(), path);
        }
    }
}

void MainWindow::startEditing() {
    qDebug() << "Start Editing - not implemented yet";
}

void MainWindow::saveDefinition(bool asNew) {
    qDebug() << "Save Definition" << (asNew ? "As" : "To current");
}

void MainWindow::saveStyle(bool asNew) {
    qDebug() << "Save Style" << (asNew ? "As" : "To current");
}

void MainWindow::showSettings() {
    SettingsDialog dlg(m_settings, this);
    connect(&dlg, &SettingsDialog::settingsApplied, this, [this](const AppSettings &s) {
        m_settings = s;
        // Apply settings
        if (s.alwaysOnTop) {
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
            show();
        } else {
            setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
            show();
        }
        saveSettings(); });
    dlg.exec();
}

void MainWindow::showLoadDialog() {
    KeyboardLoader loader(KeyboardLoader::getDefaultKeyboardsPath());
    LoadDialog dialog(&loader, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString defPath = dialog.selectedDefinitionPath();
        QString stylePath = dialog.selectedStylePath();
        if (!defPath.isEmpty()) {
            m_keyboardWidget->loadKeyboard(defPath, stylePath);
            updateLoadedKeyboard(defPath, stylePath);
            adjustSize();
        }
    }
}

void MainWindow::updateLoadedKeyboard(const QString &defPath, const QString &stylePath) {
    QFileInfo defInfo(defPath);
    QString keyboardName = defInfo.dir().dirName();
    QString category = defInfo.dir().path().section("/", -2, -2);
    m_settings.loadedCategory = category;
    m_settings.loadedKeyboard = keyboardName;

    if (!stylePath.isEmpty()) {
        QFileInfo styleInfo(stylePath);
        m_settings.loadedStyle = styleInfo.baseName();
    } else {
        m_settings.loadedStyle.clear();
    }

    saveSettings();
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragPos);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
    }
}

void MainWindow::saveSettings() {
    QJsonObject obj;
    obj["MouseSensitivity"] = m_settings.mouseSensitivity;
    obj["UpdateInterval"] = m_settings.updateInterval;
    obj["ScrollHold"] = m_settings.scrollHold;
    obj["MouseFromCenter"] = m_settings.mouseFromCenter;
    obj["PressHold"] = m_settings.pressHold;
    obj["WindowTitle"] = m_settings.windowTitle;
    obj["Capitalization"] = m_settings.capitalization;
    obj["FollowShiftForCapsInsensitive"] = m_settings.followShiftForCapsInsensitive;
    obj["FollowShiftForCapsSensitive"] = m_settings.followShiftForCapsSensitive;
    obj["TrapKeyboard"] = m_settings.trapKeyboard;
    obj["TrapMouse"] = m_settings.trapMouse;
    obj["TrapToggleKeyCode"] = m_settings.trapToggleKeyCode;
    obj["UpdateTextPosition"] = m_settings.updateTextPosition;
    obj["LoadedCategory"] = m_settings.loadedCategory;
    obj["LoadedKeyboard"] = m_settings.loadedKeyboard;
    obj["LoadedStyle"] = m_settings.loadedStyle;
    obj["LoadedGlobalStyle"] = m_settings.loadedGlobalStyle;
    obj["X"] = pos().x();
    obj["Y"] = pos().y();

    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/NohBoard.json";
    QDir().mkpath(QFileInfo(settingsPath).path()); // ensure directory exists
    QFile file(settingsPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
        file.close();
    }
}