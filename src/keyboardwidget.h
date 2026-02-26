#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include "models/keyboarddefinition.h"
#include "models/keyboardstyle.h"
#include <QMap>
#include <QMultiMap>
#include <QSet>
#include <QTimer>
#include <QWidget>

// Forward declare KeySym to avoid X11/Qt conflicts
typedef unsigned long KeySym;

class QMenu;
class InputManager;

class KeyboardWidget : public QWidget {
    Q_OBJECT

  public:
    explicit KeyboardWidget(QWidget *parent = nullptr, InputManager *inputManager = nullptr);
    ~KeyboardWidget() override;
    std::shared_ptr<ElementDefinition> findElementById(int id) const;
    QString getCurrentDefinitionPath() const { return m_currentDefinitionPath; }

  public slots:
    void onKeyStateChanged(int keyCode, bool pressed);
    void onMouseButtonChanged(int button, bool pressed);
    void loadKeyboard(const QString &defPath, const QString &stylePath = QString());
    void onMouseScrolled(int direction);
    void setEditing(bool editing);
    void moveSelectedElement(int direction);
    void removeSelectedElement();
    void editKeyboardProperties();
    void editElementProperties();
    void editKeyboardStyle();
    void editKeyStyle();
    void setKeyboardSize(int w, int h);
    void saveDefinition(bool asNew = false);
    void saveStyle(bool asNew = false);
    void updateElement(std::shared_ptr<ElementDefinition> elem);
    void onMouseMoved(float dx, float dy);

  signals:
    void settingsRequested();
    void loadRequested();
    void editRequested();
    void saveDefinitionRequested(bool asNew);
    void saveStyleRequested(bool asNew);
    void editingChanged(bool editing);
    void elementSelected(int id);
    void showKeyboardProperties(int width, int height);
    void showElementProperties(int id);
    void showKeyboardStyle(const QString &stylePath);
    void showKeyStyle(int id, const QString &stylePath);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void setupContextMenu();
    KeyboardDefinition m_definition;
    KeyboardStyle m_style;

    QSet<unsigned int> m_pressedKeys;
    QMenu *m_contextMenu = nullptr;
    bool m_dragging = false;
    QPoint m_dragPos;

    QMap<QString, QPixmap> m_imageCache;
    QString m_styleBasePath; // directory containing the current .style file
    QSet<int> m_pressedMouseButtons;
    InputManager *m_inputManager;
    QString m_imageBasePath;

    QTimer *m_smoothTimer;
    float m_smoothDx;
    float m_smoothDy;
    static constexpr float SMOOTHING_ALPHA = 0.01f; // responsiveness (0.1-0.5)
    static constexpr float DECAY_FACTOR = 0;        // per‑frame decay when idle
    static constexpr float EPSILON = 0.25f;         // stop timer when speed below this

    QSet<int> m_activeScrollDirections;
    QTimer *m_scrollTimer;
    static constexpr int SCROLL_TIMEOUT = 100; // ms

    QSet<QString> m_failedImages;

    bool m_editing = false;
    int m_selectedElementId = -1;
    QString m_currentDefinitionPath;
    QString m_currentStylePath;

    // Optimization: element bounding rects and code maps
    QMap<int, QRect> m_elementRects;            // element id -> bounding rect
    QMultiMap<int, int> m_keyCodeToElement;     // key code -> element id (for keys and scroll)
    QMultiMap<int, int> m_mouseButtonToElement; // mouse button -> element id
    QList<int> m_scrollElementIds;              // list of scroll element ids for timeout updates
    int m_speedIndicatorId;

  private slots:
    void updateSmoothing();
};

#endif // KEYBOARDWIDGET_H
