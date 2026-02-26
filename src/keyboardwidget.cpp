#include "keyboardwidget.h"
#include "input/inputmanager.h"
#include "loaders/definitionloader.h"
#include "loaders/styleloader.h"
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

KeyboardWidget::KeyboardWidget(QWidget *parent, InputManager *inputManager)
    : QWidget(parent), m_inputManager(inputManager), m_speedIndicatorId(-1) {
    setMinimumSize(800, 300);
    setupContextMenu();

    m_smoothDx = 0.0f;
    m_smoothDy = 0.0f;
    m_smoothTimer = new QTimer(this);
    m_smoothTimer->setInterval(16); // ~60 fps
    connect(m_smoothTimer, &QTimer::timeout, this, &KeyboardWidget::updateSmoothing);

    // Add this block:
    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setSingleShot(true);
    connect(m_scrollTimer, &QTimer::timeout, this, [this]() {
        m_activeScrollDirections.clear();
        // Update all scroll elements on timeout
        for (int id : m_scrollElementIds) {
            if (m_elementRects.contains(id))
                update(m_elementRects[id]);
        }
    });
}

KeyboardWidget::~KeyboardWidget() {
}

void KeyboardWidget::updateSmoothing() {
    m_smoothDx *= DECAY_FACTOR;
    m_smoothDy *= DECAY_FACTOR;
    if (qAbs(m_smoothDx) < EPSILON && qAbs(m_smoothDy) < EPSILON) {
        m_smoothTimer->stop();
        m_smoothDx = 0.0f;
        m_smoothDy = 0.0f;
    }
    // Update only the speed indicator area
    if (m_speedIndicatorId >= 0 && m_elementRects.contains(m_speedIndicatorId))
        update(m_elementRects[m_speedIndicatorId]);
}

std::shared_ptr<ElementDefinition> KeyboardWidget::findElementById(int id) const {
    for (const auto &elem : m_definition.elements) {
        if (elem->id == id) return elem;
    }
    return nullptr;
}

void KeyboardWidget::updateElement(std::shared_ptr<ElementDefinition> elem) {
    for (int i = 0; i < m_definition.elements.size(); ++i) {
        if (m_definition.elements[i]->id == elem->id) {
            m_definition.elements[i] = elem;
            update();
            return;
        }
    }
}

void KeyboardWidget::setKeyboardSize(int w, int h) {
    m_definition.width = w;
    m_definition.height = h;
    setFixedSize(w, h);
    update();
    // Optionally save to file
}

void KeyboardWidget::saveDefinition(bool asNew) {
    if (m_currentDefinitionPath.isEmpty()) return;

    QString savePath = m_currentDefinitionPath;
    if (asNew) {
        savePath = QFileDialog::getSaveFileName(this, "Save Definition As",
                                                QFileInfo(m_currentDefinitionPath).path(),
                                                "JSON files (*.json)");
        if (savePath.isEmpty()) return;
    }

    QJsonObject obj;
    obj["Version"] = m_definition.version;
    obj["Width"] = m_definition.width;
    obj["Height"] = m_definition.height;

    QJsonArray elements;
    for (const auto &elem : m_definition.elements) {
        QJsonObject elemObj;
        elemObj["Id"] = elem->id;
        elemObj["__type"] = [&]() -> QString {
            if (elem->type == ElementType::KeyboardKey) return "KeyboardKey";
            if (elem->type == ElementType::MouseKey) return "MouseKey";
            if (elem->type == ElementType::MouseScroll) return "MouseScroll";
            if (elem->type == ElementType::MouseSpeedIndicator) return "MouseSpeedIndicator";
            return "";
        }();

        // Boundaries
        QJsonArray boundsArr;
        if (elem->type != ElementType::MouseSpeedIndicator) {
            // All except speed indicator have boundaries
            // We need to cast to appropriate type to access boundaries
            if (elem->type == ElementType::KeyboardKey) {
                auto k = std::dynamic_pointer_cast<KeyboardKeyDefinition>(elem);
                for (const auto &pt : k->boundaries) {
                    QJsonObject ptObj;
                    ptObj["X"] = pt.x;
                    ptObj["Y"] = pt.y;
                    boundsArr.append(ptObj);
                }
            } else if (elem->type == ElementType::MouseKey) {
                auto m = std::dynamic_pointer_cast<MouseKeyDefinition>(elem);
                for (const auto &pt : m->boundaries) {
                    QJsonObject ptObj;
                    ptObj["X"] = pt.x;
                    ptObj["Y"] = pt.y;
                    boundsArr.append(ptObj);
                }
            } else if (elem->type == ElementType::MouseScroll) {
                auto s = std::dynamic_pointer_cast<MouseScrollDefinition>(elem);
                for (const auto &pt : s->boundaries) {
                    QJsonObject ptObj;
                    ptObj["X"] = pt.x;
                    ptObj["Y"] = pt.y;
                    boundsArr.append(ptObj);
                }
            }
            elemObj["Boundaries"] = boundsArr;
        }

        // KeyCodes
        QJsonArray keyCodesArr;
        for (int code : elem->keyCodes)
            keyCodesArr.append(code);
        elemObj["KeyCodes"] = keyCodesArr;

        // TextPosition (if present)
        if (elem->type != ElementType::MouseSpeedIndicator) {
            // All except speed indicator have textPosition
            QJsonObject tpObj;
            tpObj["X"] = elem->textPosition.x;
            tpObj["Y"] = elem->textPosition.y;
            elemObj["TextPosition"] = tpObj;
        }

        // Text
        if (!elem->text.isEmpty())
            elemObj["Text"] = elem->text;

        // Type-specific fields
        if (elem->type == ElementType::KeyboardKey) {
            auto k = std::dynamic_pointer_cast<KeyboardKeyDefinition>(elem);
            elemObj["ShiftText"] = k->shiftText;
            elemObj["ChangeOnCaps"] = k->changeOnCaps;
        } else if (elem->type == ElementType::MouseSpeedIndicator) {
            auto m = std::dynamic_pointer_cast<MouseSpeedIndicatorDefinition>(elem);
            QJsonObject locObj;
            locObj["X"] = m->location.x;
            locObj["Y"] = m->location.y;
            elemObj["Location"] = locObj;
            elemObj["Radius"] = m->radius;
        }

        elements.append(elemObj);
    }
    obj["Elements"] = elements;

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
        file.close();
        if (!asNew) {
            // Keep using same path
        } else {
            m_currentDefinitionPath = savePath;
        }
        qDebug() << "Definition saved to" << savePath;
    } else {
        qWarning() << "Failed to save definition to" << savePath;
    }
}

void KeyboardWidget::saveStyle(bool asNew) {
    if (m_currentStylePath.isEmpty()) return;

    QString savePath = m_currentStylePath;
    if (asNew) {
        savePath = QFileDialog::getSaveFileName(this, "Save Style As",
                                                QFileInfo(m_currentStylePath).path(),
                                                "Style files (*.style)");
        if (savePath.isEmpty()) return;
    }

    if (StyleLoader::save(m_style, savePath)) {
        if (asNew)
            m_currentStylePath = savePath;
        qDebug() << "Style saved to" << savePath;
    } else {
        qWarning() << "Failed to save style to" << savePath;
    }
}

void KeyboardWidget::loadKeyboard(const QString &defPath, const QString &stylePath) {
    auto defOpt = DefinitionLoader::load(defPath);
    if (!defOpt) {
        qWarning() << "Failed to load keyboard definition:" << defPath;
        return;
    }

    m_definition = *defOpt;
    m_currentDefinitionPath = defPath;
    m_currentStylePath = stylePath;

    setFixedSize(m_definition.width, m_definition.height);

    if (!stylePath.isEmpty() && QFile::exists(stylePath)) {
        auto styleOpt = StyleLoader::load(stylePath);
        if (styleOpt) {
            m_style = *styleOpt;
            m_styleBasePath = QFileInfo(stylePath).path();
            QDir styleDir(m_styleBasePath);
            styleDir.cdUp();
            m_imageBasePath = styleDir.filePath("images");
        } else {
            m_style = StyleLoader::defaultStyle();
            m_styleBasePath.clear();
            m_imageBasePath.clear();
        }
    } else {
        m_style = StyleLoader::defaultStyle();
        m_styleBasePath.clear();
        m_imageBasePath.clear();
    }

    // Build optimization maps
    m_elementRects.clear();
    m_keyCodeToElement.clear();
    m_mouseButtonToElement.clear();
    m_scrollElementIds.clear();
    m_speedIndicatorId = -1;

    for (const auto &elem : m_definition.elements) {
        // Compute bounding rect
        QRect rect;
        if (elem->type == ElementType::MouseSpeedIndicator) {
            auto speed = std::dynamic_pointer_cast<MouseSpeedIndicatorDefinition>(elem);
            if (speed) {
                rect = QRect(speed->location.x - speed->radius,
                             speed->location.y - speed->radius,
                             2 * speed->radius, 2 * speed->radius);
                m_speedIndicatorId = elem->id;
            }
        } else {
            QPolygon polygon;
            if (elem->type == ElementType::KeyboardKey) {
                auto k = std::dynamic_pointer_cast<KeyboardKeyDefinition>(elem);
                if (k) {
                    for (const auto &pt : k->boundaries)
                        polygon << QPoint(pt.x, pt.y);
                }
            } else if (elem->type == ElementType::MouseKey) {
                auto m = std::dynamic_pointer_cast<MouseKeyDefinition>(elem);
                if (m) {
                    for (const auto &pt : m->boundaries)
                        polygon << QPoint(pt.x, pt.y);
                }
            } else if (elem->type == ElementType::MouseScroll) {
                auto s = std::dynamic_pointer_cast<MouseScrollDefinition>(elem);
                if (s) {
                    for (const auto &pt : s->boundaries)
                        polygon << QPoint(pt.x, pt.y);
                }
            }
            rect = polygon.boundingRect();
        }
        if (!rect.isNull()) {
            m_elementRects[elem->id] = rect;
        }

        // Populate code maps
        for (int code : elem->keyCodes) {
            if (elem->type == ElementType::KeyboardKey) {
                m_keyCodeToElement.insert(code, elem->id);
            } else if (elem->type == ElementType::MouseKey) {
                m_mouseButtonToElement.insert(code, elem->id);
            } else if (elem->type == ElementType::MouseScroll) {
                m_keyCodeToElement.insert(code, elem->id); // scroll codes also go here
                m_scrollElementIds.append(elem->id);
            }
        }
    }

    update();
}

void KeyboardWidget::editKeyboardProperties() {
    emit showKeyboardProperties(m_definition.width, m_definition.height);
}

void KeyboardWidget::editElementProperties() {
    if (m_selectedElementId >= 0)
        emit showElementProperties(m_selectedElementId);
}

void KeyboardWidget::editKeyboardStyle() {
    emit showKeyboardStyle(m_currentStylePath);
}

void KeyboardWidget::editKeyStyle() {
    if (m_selectedElementId >= 0)
        emit showKeyStyle(m_selectedElementId, m_currentStylePath);
}

void KeyboardWidget::removeSelectedElement() {
    if (m_selectedElementId < 0) return;
    for (int i = 0; i < m_definition.elements.size(); ++i) {
        if (m_definition.elements[i]->id == m_selectedElementId) {
            m_definition.elements.removeAt(i);
            m_selectedElementId = -1;
            update();
            return;
        }
    }
}

void KeyboardWidget::moveSelectedElement(int direction) {
    if (m_selectedElementId < 0) return;
    // Find index
    int index = -1;
    for (int i = 0; i < m_definition.elements.size(); ++i) {
        if (m_definition.elements[i]->id == m_selectedElementId) {
            index = i;
            break;
        }
    }
    if (index < 0) return;
    int newIndex = index;
    if (direction == -1) newIndex = index - 1;                            // up
    else if (direction == 1) newIndex = index + 1;                        // down
    else if (direction == -2) newIndex = 0;                               // top
    else if (direction == 2) newIndex = m_definition.elements.size() - 1; // bottom
    if (newIndex < 0 || newIndex >= m_definition.elements.size()) return;
    auto elem = m_definition.elements.takeAt(index);
    m_definition.elements.insert(newIndex, elem);
    update();
    // Optionally save to file
}

void KeyboardWidget::setEditing(bool editing) {
    m_editing = editing;
    m_selectedElementId = -1;
    setupContextMenu();
    emit editingChanged(editing);
}

void KeyboardWidget::setupContextMenu() {
    if (m_contextMenu) delete m_contextMenu;
    m_contextMenu = new QMenu(this);

    if (m_editing) {
        QAction *stopEditAction = m_contextMenu->addAction("Stop Editing");
        connect(stopEditAction, &QAction::triggered, this, [this]() { setEditing(false); });

        // Move submenu
        QMenu *moveMenu = m_contextMenu->addMenu("Move");
        QAction *moveTop = moveMenu->addAction("Move to top");
        QAction *moveUp = moveMenu->addAction("Move up");
        QAction *moveDown = moveMenu->addAction("Move down");
        QAction *moveBottom = moveMenu->addAction("Move to bottom");
        connect(moveTop, &QAction::triggered, this, [this]() { moveSelectedElement(-2); });
        connect(moveUp, &QAction::triggered, this, [this]() { moveSelectedElement(-1); });
        connect(moveDown, &QAction::triggered, this, [this]() { moveSelectedElement(1); });
        connect(moveBottom, &QAction::triggered, this, [this]() { moveSelectedElement(2); });

        QAction *removeAction = m_contextMenu->addAction("Remove element");
        connect(removeAction, &QAction::triggered, this, &KeyboardWidget::removeSelectedElement);

        QAction *kbPropsAction = m_contextMenu->addAction("Keyboard properties");
        connect(kbPropsAction, &QAction::triggered, this, &KeyboardWidget::editKeyboardProperties);

        QAction *elemPropsAction = m_contextMenu->addAction("Element properties");
        connect(elemPropsAction, &QAction::triggered, this, &KeyboardWidget::editElementProperties);

        QAction *kbStyleAction = m_contextMenu->addAction("Keyboard style");
        connect(kbStyleAction, &QAction::triggered, this, &KeyboardWidget::editKeyboardStyle);

        QAction *keyStyleAction = m_contextMenu->addAction("Key style");
        connect(keyStyleAction, &QAction::triggered, this, &KeyboardWidget::editKeyStyle);

        m_contextMenu->addSeparator();

        // Save Definition submenu
        QMenu *saveDefMenu = m_contextMenu->addMenu("Save Definition");
        QAction *saveDefToCurrent = saveDefMenu->addAction("Save To 'current keyboard'");
        QAction *saveDefAs = saveDefMenu->addAction("Save As");
        connect(saveDefToCurrent, &QAction::triggered, this, [this]() { saveDefinition(false); });
        connect(saveDefAs, &QAction::triggered, this, [this]() { saveDefinition(true); });

        // Save Style submenu
        QMenu *saveStyleMenu = m_contextMenu->addMenu("Save Style");
        QAction *saveStyleToCurrent = saveStyleMenu->addAction("Save To 'current style'");
        QAction *saveStyleAs = saveStyleMenu->addAction("Save As");
        connect(saveStyleToCurrent, &QAction::triggered, this, [this]() { saveStyle(false); });
        connect(saveStyleAs, &QAction::triggered, this, [this]() { saveStyle(true); });

    } else {
        // Normal menu
        QAction *loadAction = m_contextMenu->addAction("Load Keyboard...");
        connect(loadAction, &QAction::triggered, this, &KeyboardWidget::loadRequested);

        QAction *settingsAction = m_contextMenu->addAction("Settings");
        connect(settingsAction, &QAction::triggered, this, &KeyboardWidget::settingsRequested);

        QAction *startEditAction = m_contextMenu->addAction("Start Editing");
        connect(startEditAction, &QAction::triggered, this, [this]() { setEditing(true); });

        m_contextMenu->addSeparator();

        // Save Definition submenu
        QMenu *saveDefMenu = m_contextMenu->addMenu("Save Definition");
        QAction *saveDefToCurrent = saveDefMenu->addAction("Save To 'current keyboard'");
        QAction *saveDefAs = saveDefMenu->addAction("Save As");
        connect(saveDefToCurrent, &QAction::triggered, this, [this]() { saveDefinition(false); });
        connect(saveDefAs, &QAction::triggered, this, [this]() { saveDefinition(true); });

        // Save Style submenu
        QMenu *saveStyleMenu = m_contextMenu->addMenu("Save Style");
        QAction *saveStyleToCurrent = saveStyleMenu->addAction("Save To 'current style'");
        QAction *saveStyleAs = saveStyleMenu->addAction("Save As");
        connect(saveStyleToCurrent, &QAction::triggered, this, [this]() { saveStyle(false); });
        connect(saveStyleAs, &QAction::triggered, this, [this]() { saveStyle(true); });

        m_contextMenu->addSeparator();

        QAction *exitAction = m_contextMenu->addAction("Exit");
        connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    }
}
void KeyboardWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(rect(), m_style.backgroundColor);

    QRect clip = event->rect();

    for (const auto &elem : m_definition.elements) {

        // Skip if element is outside the clip region
        if (m_elementRects.contains(elem->id) && !m_elementRects[elem->id].intersects(clip))
            continue;

        // ----- Keyboard Key -----
        if (elem->type == ElementType::KeyboardKey) {
            auto keyElem = std::dynamic_pointer_cast<KeyboardKeyDefinition>(elem);
            if (!keyElem) continue;

            bool pressed = false;
            for (int code : keyElem->keyCodes)
                if (m_pressedKeys.contains(static_cast<unsigned int>(code))) {
                    pressed = true;
                    break;
                }

            KeyStyle keyStyle = m_style.defaultKeyStyle;
            if (m_style.keyElementStyles.contains(keyElem->id))
                keyStyle = m_style.keyElementStyles[keyElem->id];
            const KeySubStyle &substyle = pressed ? keyStyle.pressed : keyStyle.loose;

            QPolygon polygon;
            for (const auto &pt : keyElem->boundaries)
                polygon << QPoint(pt.x, pt.y);

            // Draw background (image or color)
            if (!substyle.backgroundImageFileName.isEmpty()) {
                QString filename = substyle.backgroundImageFileName;
                if (filename.startsWith("./"))
                    filename = filename.mid(2);

                QString imagePath;
                QPixmap pixmap;

                // Try image base path first
                if (!m_imageBasePath.isEmpty()) {
                    imagePath = QDir(m_imageBasePath).filePath(filename);
                    // qDebug() << "Attempting to load image from:" << imagePath;
                    // if (QFile::exists(imagePath))
                    //     qDebug() << "  File exists.";
                    // else
                    //     qDebug() << "  File does NOT exist!";

                    if (m_failedImages.contains(imagePath)) {
                        qDebug() << "  Previously failed, skipping.";
                    } else if (m_imageCache.contains(imagePath)) {
                        pixmap = m_imageCache[imagePath];
                        // qDebug() << "  Using cached pixmap, size:" << pixmap.size();
                    } else {
                        if (imagePath.endsWith(".svg", Qt::CaseInsensitive)) {
                            QSvgRenderer renderer(imagePath);
                            if (renderer.isValid()) {
                                QSize size = polygon.boundingRect().size();
                                QPixmap svgPixmap(size);
                                svgPixmap.fill(Qt::transparent);
                                QPainter svgPainter(&svgPixmap);
                                renderer.render(&svgPainter);
                                pixmap = svgPixmap;
                                qDebug() << "  Rendered SVG, size:" << pixmap.size();
                            } else {
                                qDebug() << "  SVG renderer invalid";
                            }
                        } else {
                            pixmap.load(imagePath);
                        }
                        if (!pixmap.isNull()) {
                            m_imageCache.insert(imagePath, pixmap);
                            qDebug() << "  Image loaded successfully, size:" << pixmap.size();
                        } else {
                            m_failedImages.insert(imagePath);
                            qDebug() << "  Failed to load image (pixmap is null).";
                            // Get more info
                            QImageReader reader(imagePath);
                            qDebug() << "  QImageReader error:" << reader.errorString();
                            qDebug() << "  Supported formats:" << QImageReader::supportedImageFormats();
                        }
                    }
                }

                // If not found, fallback to style base path (similar structure)
                if (pixmap.isNull() && !m_styleBasePath.isEmpty()) {
                    imagePath = QDir(m_styleBasePath).filePath(filename);
                    qDebug() << "Fallback to style path:" << imagePath;
                    if (QFile::exists(imagePath))
                        qDebug() << "  File exists.";
                    else
                        qDebug() << "  File does NOT exist!";

                    if (m_failedImages.contains(imagePath)) {
                        qDebug() << "  Previously failed, skipping.";
                    } else if (m_imageCache.contains(imagePath)) {
                        pixmap = m_imageCache[imagePath];
                        qDebug() << "  Using cached pixmap, size:" << pixmap.size();
                    } else {
                        if (imagePath.endsWith(".svg", Qt::CaseInsensitive)) {
                            QSvgRenderer renderer(imagePath);
                            if (renderer.isValid()) {
                                QSize size = polygon.boundingRect().size();
                                QPixmap svgPixmap(size);
                                svgPixmap.fill(Qt::transparent);
                                QPainter svgPainter(&svgPixmap);
                                renderer.render(&svgPainter);
                                pixmap = svgPixmap;
                                qDebug() << "  Rendered SVG, size:" << pixmap.size();
                            } else {
                                qDebug() << "  SVG renderer invalid";
                            }
                        } else {
                            pixmap.load(imagePath);
                        }
                        if (!pixmap.isNull()) {
                            m_imageCache.insert(imagePath, pixmap);
                            qDebug() << "  Image loaded successfully, size:" << pixmap.size();
                        } else {
                            m_failedImages.insert(imagePath);
                            qDebug() << "  Failed to load image (pixmap is null).";
                            QImageReader reader(imagePath);
                            qDebug() << "  QImageReader error:" << reader.errorString();
                            qDebug() << "  Supported formats:" << QImageReader::supportedImageFormats();
                        }
                    }
                }

                if (!pixmap.isNull()) {
                    QRect targetRect = polygon.boundingRect();
                    // qDebug() << "  Drawing to rect:" << targetRect;
                    if (pixmap.size() != targetRect.size()) {
                        pixmap = pixmap.scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                        // qDebug() << "  Scaled pixmap to" << pixmap.size();
                    }
                    painter.drawPixmap(targetRect, pixmap);

                    if (substyle.showOutline) {
                        painter.setBrush(Qt::NoBrush);
                        painter.setPen(QPen(substyle.outline, substyle.outlineWidth));
                        painter.drawPolygon(polygon);
                    }
                } else {
                    // fallback to color
                    qDebug() << "  Using color fallback for key" << keyElem->id;
                    painter.setBrush(substyle.background);
                    painter.setPen(substyle.showOutline ? QPen(substyle.outline, substyle.outlineWidth) : Qt::NoPen);
                    painter.drawPolygon(polygon);
                }
            } else {
                // No image
                painter.setBrush(substyle.background);
                painter.setPen(substyle.showOutline ? QPen(substyle.outline, substyle.outlineWidth) : Qt::NoPen);
                painter.drawPolygon(polygon);
            }

            // Determine display text based on shift state
            bool shiftPressed = m_pressedKeys.contains(0xA0) || m_pressedKeys.contains(0xA1);
            QString displayText = keyElem->text;
            if (shiftPressed && !keyElem->shiftText.isEmpty())
                displayText = keyElem->shiftText;
            // (Caps lock handling can be added later)

            if (!displayText.isEmpty()) {
                painter.setPen(substyle.text);
                painter.setFont(substyle.font.toQFont());
                QFontMetrics fm(painter.font());
                QRect textRect = fm.boundingRect(displayText);
                textRect.moveCenter(QPoint(keyElem->textPosition.x, keyElem->textPosition.y));
                painter.drawText(textRect, Qt::AlignCenter, displayText);
            }
        }

        // ----- Mouse Key -----
        else if (elem->type == ElementType::MouseKey) {
            auto mouseElem = std::dynamic_pointer_cast<MouseKeyDefinition>(elem);
            if (!mouseElem) continue;

            bool pressed = false;
            for (int code : mouseElem->keyCodes)
                if (m_pressedMouseButtons.contains(code)) {
                    pressed = true;
                    break;
                }

            KeyStyle keyStyle = m_style.defaultKeyStyle;
            if (m_style.keyElementStyles.contains(mouseElem->id))
                keyStyle = m_style.keyElementStyles[mouseElem->id];
            const KeySubStyle &substyle = pressed ? keyStyle.pressed : keyStyle.loose;

            QPolygon polygon;
            for (const auto &pt : mouseElem->boundaries)
                polygon << QPoint(pt.x, pt.y);

            // Draw background (image or color)
            if (!substyle.backgroundImageFileName.isEmpty()) {
                QString filename = substyle.backgroundImageFileName;
                if (filename.startsWith("./"))
                    filename = filename.mid(2);

                QString imagePath;
                QPixmap pixmap;

                // Try image base path first
                if (!m_imageBasePath.isEmpty()) {
                    imagePath = QDir(m_imageBasePath).filePath(filename);
                    if (!m_failedImages.contains(imagePath)) {
                        if (m_imageCache.contains(imagePath))
                            pixmap = m_imageCache[imagePath];
                        else {
                            if (imagePath.endsWith(".svg", Qt::CaseInsensitive)) {
                                QSvgRenderer renderer(imagePath);
                                if (renderer.isValid()) {
                                    QSize size = polygon.boundingRect().size();
                                    QPixmap svgPixmap(size);
                                    svgPixmap.fill(Qt::transparent);
                                    QPainter svgPainter(&svgPixmap);
                                    renderer.render(&svgPainter);
                                    pixmap = svgPixmap;
                                }
                            } else {
                                pixmap.load(imagePath);
                            }
                            if (!pixmap.isNull())
                                m_imageCache.insert(imagePath, pixmap);
                            else
                                m_failedImages.insert(imagePath);
                        }
                    }
                }

                // Fallback to style base path
                if (pixmap.isNull() && !m_styleBasePath.isEmpty()) {
                    imagePath = QDir(m_styleBasePath).filePath(filename);
                    if (!m_failedImages.contains(imagePath)) {
                        if (m_imageCache.contains(imagePath))
                            pixmap = m_imageCache[imagePath];
                        else {
                            if (imagePath.endsWith(".svg", Qt::CaseInsensitive)) {
                                QSvgRenderer renderer(imagePath);
                                if (renderer.isValid()) {
                                    QSize size = polygon.boundingRect().size();
                                    QPixmap svgPixmap(size);
                                    svgPixmap.fill(Qt::transparent);
                                    QPainter svgPainter(&svgPixmap);
                                    renderer.render(&svgPainter);
                                    pixmap = svgPixmap;
                                }
                            } else {
                                pixmap.load(imagePath);
                            }
                            if (!pixmap.isNull())
                                m_imageCache.insert(imagePath, pixmap);
                            else
                                m_failedImages.insert(imagePath);
                        }
                    }
                }

                if (!pixmap.isNull()) {
                    QRect targetRect = polygon.boundingRect();
                    if (pixmap.size() != targetRect.size())
                        pixmap = pixmap.scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    painter.drawPixmap(targetRect, pixmap);

                    if (substyle.showOutline) {
                        painter.setBrush(Qt::NoBrush);
                        painter.setPen(QPen(substyle.outline, substyle.outlineWidth));
                        painter.drawPolygon(polygon);
                    }
                } else {
                    painter.setBrush(substyle.background);
                    painter.setPen(substyle.showOutline ? QPen(substyle.outline, substyle.outlineWidth) : Qt::NoPen);
                    painter.drawPolygon(polygon);
                }
            } else {
                painter.setBrush(substyle.background);
                painter.setPen(substyle.showOutline ? QPen(substyle.outline, substyle.outlineWidth) : Qt::NoPen);
                painter.drawPolygon(polygon);
            }

            // Draw text
            if (!mouseElem->text.isEmpty()) {
                painter.setPen(substyle.text);
                painter.setFont(substyle.font.toQFont());
                QFontMetrics fm(painter.font());
                QRect textRect = fm.boundingRect(mouseElem->text);
                textRect.moveCenter(polygon.boundingRect().center());
                painter.drawText(textRect, Qt::AlignCenter, mouseElem->text);
            }
        }

        // ----- Mouse Scroll -----
        else if (elem->type == ElementType::MouseScroll) {
            auto scrollElem = std::dynamic_pointer_cast<MouseScrollDefinition>(elem);
            if (!scrollElem) continue;

            bool active = false;
            if (!scrollElem->keyCodes.isEmpty())
                active = m_activeScrollDirections.contains(scrollElem->keyCodes.first());

            KeyStyle keyStyle = m_style.defaultKeyStyle;
            if (m_style.keyElementStyles.contains(scrollElem->id))
                keyStyle = m_style.keyElementStyles[scrollElem->id];
            const KeySubStyle &substyle = active ? keyStyle.pressed : keyStyle.loose;

            QPolygon polygon;
            for (const auto &pt : scrollElem->boundaries)
                polygon << QPoint(pt.x, pt.y);

            // Draw background (image or color) – same as MouseKey
            if (!substyle.backgroundImageFileName.isEmpty()) {
                QString filename = substyle.backgroundImageFileName;
                if (filename.startsWith("./"))
                    filename = filename.mid(2);

                QString imagePath;
                QPixmap pixmap;

                // Try image base path first
                if (!m_imageBasePath.isEmpty()) {
                    imagePath = QDir(m_imageBasePath).filePath(filename);
                    if (!m_failedImages.contains(imagePath)) {
                        if (m_imageCache.contains(imagePath))
                            pixmap = m_imageCache[imagePath];
                        else {
                            if (imagePath.endsWith(".svg", Qt::CaseInsensitive)) {
                                QSvgRenderer renderer(imagePath);
                                if (renderer.isValid()) {
                                    QSize size = polygon.boundingRect().size();
                                    QPixmap svgPixmap(size);
                                    svgPixmap.fill(Qt::transparent);
                                    QPainter svgPainter(&svgPixmap);
                                    renderer.render(&svgPainter);
                                    pixmap = svgPixmap;
                                }
                            } else {
                                pixmap.load(imagePath);
                            }
                            if (!pixmap.isNull())
                                m_imageCache.insert(imagePath, pixmap);
                            else
                                m_failedImages.insert(imagePath);
                        }
                    }
                }

                // Fallback to style base path
                if (pixmap.isNull() && !m_styleBasePath.isEmpty()) {
                    imagePath = QDir(m_styleBasePath).filePath(filename);
                    if (!m_failedImages.contains(imagePath)) {
                        if (m_imageCache.contains(imagePath))
                            pixmap = m_imageCache[imagePath];
                        else {
                            if (imagePath.endsWith(".svg", Qt::CaseInsensitive)) {
                                QSvgRenderer renderer(imagePath);
                                if (renderer.isValid()) {
                                    QSize size = polygon.boundingRect().size();
                                    QPixmap svgPixmap(size);
                                    svgPixmap.fill(Qt::transparent);
                                    QPainter svgPainter(&svgPixmap);
                                    renderer.render(&svgPainter);
                                    pixmap = svgPixmap;
                                }
                            } else {
                                pixmap.load(imagePath);
                            }
                            if (!pixmap.isNull())
                                m_imageCache.insert(imagePath, pixmap);
                            else
                                m_failedImages.insert(imagePath);
                        }
                    }
                }

                if (!pixmap.isNull()) {
                    QRect targetRect = polygon.boundingRect();
                    if (pixmap.size() != targetRect.size())
                        pixmap = pixmap.scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    painter.drawPixmap(targetRect, pixmap);

                    if (substyle.showOutline) {
                        painter.setBrush(Qt::NoBrush);
                        painter.setPen(QPen(substyle.outline, substyle.outlineWidth));
                        painter.drawPolygon(polygon);
                    }
                } else {
                    painter.setBrush(substyle.background);
                    painter.setPen(substyle.showOutline ? QPen(substyle.outline, substyle.outlineWidth) : Qt::NoPen);
                    painter.drawPolygon(polygon);
                }
            } else {
                painter.setBrush(substyle.background);
                painter.setPen(substyle.showOutline ? QPen(substyle.outline, substyle.outlineWidth) : Qt::NoPen);
                painter.drawPolygon(polygon);
            }

            // Draw text
            if (!scrollElem->text.isEmpty()) {
                painter.setPen(substyle.text);
                painter.setFont(substyle.font.toQFont());
                QFontMetrics fm(painter.font());
                QRect textRect = fm.boundingRect(scrollElem->text);
                textRect.moveCenter(polygon.boundingRect().center());
                painter.drawText(textRect, Qt::AlignCenter, scrollElem->text);
            }
        }

        // ----- Mouse Speed Indicator -----
        else if (elem->type == ElementType::MouseSpeedIndicator) {
            auto speedElem = std::dynamic_pointer_cast<MouseSpeedIndicatorDefinition>(elem);

            if (!speedElem) continue;
            if (!m_inputManager) continue;

            QPointF center(speedElem->location.x, speedElem->location.y);
            int radius = speedElem->radius;

            painter.setBrush(m_style.defaultMouseIndicatorStyle.outerColor);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(center, radius, radius);

            QPointF rawSpeed = m_inputManager->mouseSpeed();
            float rawDx = rawSpeed.x();
            float rawDy = rawSpeed.y();

            // Update smoothed values
            m_smoothDx = SMOOTHING_ALPHA * rawDx + (1.0f - SMOOTHING_ALPHA) * m_smoothDx;
            m_smoothDy = SMOOTHING_ALPHA * rawDy + (1.0f - SMOOTHING_ALPHA) * m_smoothDy;

            const float RESTART_EPSILON = 1e-6f;
            if (qAbs(m_smoothDx) > RESTART_EPSILON || qAbs(m_smoothDy) > RESTART_EPSILON) {
                if (!m_smoothTimer->isActive())
                    m_smoothTimer->start();
            }

            float maxDisp = radius - 5.0f;
            float dispX = m_smoothDx * maxDisp;
            float dispY = m_smoothDy * maxDisp;
            float dispLength = qSqrt(dispX * dispX + dispY * dispY);

            if (dispLength > maxDisp && dispLength > 0.001f) {
                dispX = dispX * maxDisp / dispLength;
                dispY = dispY * maxDisp / dispLength;
            }

            QPointF dotPos = center + QPointF(dispX, dispY);

            float rawSpeedVal = qSqrt(rawDx * rawDx + rawDy * rawDy);
            float threshold = 0.3f;
            QColor dotColor = (rawSpeedVal < threshold) ? m_style.defaultMouseIndicatorStyle.outerColor : m_style.defaultMouseIndicatorStyle.innerColor;

            painter.setBrush(dotColor);
            painter.drawEllipse(dotPos, 5, 5);
        }
    }
}

void KeyboardWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        if (m_editing) {
            // Find element under cursor
            QPoint pos = event->pos();
            m_selectedElementId = -1;
            for (const auto &elem : m_definition.elements) {
                // Build polygon for this element
                QPolygon polygon;
                if (elem->type == ElementType::KeyboardKey) {
                    auto keyElem = std::dynamic_pointer_cast<KeyboardKeyDefinition>(elem);
                    if (keyElem) {
                        for (const auto &pt : keyElem->boundaries)
                            polygon << QPoint(pt.x, pt.y);
                    }
                } else if (elem->type == ElementType::MouseKey) {
                    auto mouseElem = std::dynamic_pointer_cast<MouseKeyDefinition>(elem);
                    if (mouseElem) {
                        for (const auto &pt : mouseElem->boundaries)
                            polygon << QPoint(pt.x, pt.y);
                    }
                } else if (elem->type == ElementType::MouseScroll) {
                    auto scrollElem = std::dynamic_pointer_cast<MouseScrollDefinition>(elem);
                    if (scrollElem) {
                        for (const auto &pt : scrollElem->boundaries)
                            polygon << QPoint(pt.x, pt.y);
                    }
                } // MouseSpeedIndicator has no polygon
                if (!polygon.isEmpty() && polygon.containsPoint(pos, Qt::OddEvenFill)) {
                    m_selectedElementId = elem->id;
                    emit elementSelected(elem->id);
                    break;
                }
            }
        }
        m_contextMenu->exec(event->globalPosition().toPoint());
    }
    // Left button ignored (for window dragging)
}

void KeyboardWidget::mouseMoveEvent(QMouseEvent *event) {
    // if (m_dragging) {
    //     move(event->globalPosition().toPoint() - m_dragPos);
    // }
}

void KeyboardWidget::mouseReleaseEvent(QMouseEvent *event) {
    // if (event->button() == Qt::LeftButton) {
    //     m_dragging = false;
    // }
}

void KeyboardWidget::onKeyStateChanged(int keyCode, bool pressed) {
    bool changed = false;
    if (pressed) {
        if (!m_pressedKeys.contains(keyCode)) {
            m_pressedKeys.insert(keyCode);
            changed = true;
        }
    } else {
        if (m_pressedKeys.remove(keyCode))
            changed = true;
    }
    if (!changed) return;

    QList<int> ids = m_keyCodeToElement.values(keyCode);
    for (int id : ids) {
        if (m_elementRects.contains(id))
            update(m_elementRects[id]);
    }
}

void KeyboardWidget::onMouseButtonChanged(int button, bool pressed) {
    if (pressed)
        m_pressedMouseButtons.insert(button);
    else
        m_pressedMouseButtons.remove(button);

    QList<int> ids = m_mouseButtonToElement.values(button);
    for (int id : ids) {
        if (m_elementRects.contains(id))
            update(m_elementRects[id]);
    }
}

void KeyboardWidget::onMouseScrolled(int direction) {
    m_activeScrollDirections.insert(direction);
    m_scrollTimer->start(SCROLL_TIMEOUT);
    QList<int> ids = m_keyCodeToElement.values(direction);
    for (int id : ids) {
        if (m_elementRects.contains(id))
            update(m_elementRects[id]);
    }
}

void KeyboardWidget::onMouseMoved(float, float) {
    if (m_speedIndicatorId >= 0 && m_elementRects.contains(m_speedIndicatorId))
        update(m_elementRects[m_speedIndicatorId]);
}
