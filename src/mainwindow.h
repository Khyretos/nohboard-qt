#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "input/inputmanager.h"
#include "models/settings.h"
#include <QMainWindow>
#include <QPoint>

class KeyboardWidget;
class InputHook;
class QMouseEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(const AppSettings &settings, QWidget *parent = nullptr);
    ~MainWindow() override;

  public slots:
    void saveSettings();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

  private slots:
    void showSettings();
    void showLoadDialog();
    void startEditing();
    void saveDefinition(bool asNew);
    void saveStyle(bool asNew);
    void onElementSelected(int id);
    void onShowKeyboardProperties(int width, int height);
    void onShowElementProperties(int id);
    void onShowKeyboardStyle(const QString &path);
    void onShowKeyStyle(int id, const QString &path);
    void updateLoadedKeyboard(const QString &defPath, const QString &stylePath);

  private:
    void setupUi();
    void setupConnections();

    InputManager *m_inputManager;     // now first
    KeyboardWidget *m_keyboardWidget; // now second
    bool m_dragging = false;
    QPoint m_dragPos;
    AppSettings m_settings;
};

#endif // MAINWINDOW_H
