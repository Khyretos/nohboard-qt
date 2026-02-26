#include "styleloader.h"
#include "../logger.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static const QString COMP = "StyleLoader";

static QJsonObject colorToJson(const QColor &c) {
    QJsonObject obj;
    obj["Red"] = c.red();
    obj["Green"] = c.green();
    obj["Blue"] = c.blue();
    return obj;
}

static QJsonObject fontToJson(const NohFont &f) {
    QJsonObject obj;
    obj["FontFamily"] = f.fontFamily;
    obj["Size"] = static_cast<double>(f.size);
    obj["Style"] = f.style;
    return obj;
}

std::optional<KeyboardStyle> StyleLoader::load(const QString &filePath) {
    LOG_INFO(COMP, QString("🎨 Loading style: %1").arg(filePath));

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_WARN(COMP, QString("⚠️  Cannot open style file: %1 — using defaults").arg(filePath));
        return std::nullopt;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        LOG_ERROR(COMP, QString("❌ JSON parse error: %1").arg(parseError.errorString()));
        return std::nullopt;
    }

    if (!doc.isObject()) {
        LOG_ERROR(COMP, "❌ Style JSON root is not an object");
        return std::nullopt;
    }

    QJsonObject root = doc.object();
    KeyboardStyle style;

    // Background
    if (root.contains("BackgroundColor"))
        style.backgroundColor = parseColor(root.value("BackgroundColor").toObject());
    style.backgroundImageFileName = root.value("BackgroundImageFileName").toString();

    LOG_DEBUG(COMP, QString("🖼️  Background color: %1").arg(style.backgroundColor.name()));

    // Default key style
    if (root.contains("DefaultKeyStyle"))
        style.defaultKeyStyle = parseKeyStyle(root.value("DefaultKeyStyle").toObject());

    // Default mouse indicator style
    if (root.contains("DefaultMouseSpeedIndicatorStyle"))
        style.defaultMouseIndicatorStyle = parseMouseStyle(root.value("DefaultMouseSpeedIndicatorStyle").toObject());

    // Element-specific styles (supports both object and array formats)
    QJsonValue elemStylesVal = root.value("ElementStyles");
    if (elemStylesVal.isArray()) {
        QJsonArray elemStylesArray = elemStylesVal.toArray();
        for (const QJsonValue &val : elemStylesArray) {
            QJsonObject item = val.toObject();
            int elemId = item.value("Key").toInt();
            QJsonObject styleObj = item.value("Value").toObject();

            // Detect if it's a mouse indicator or key style
            if (styleObj.contains("InnerColor") || styleObj.contains("OuterColor")) {
                style.mouseIndicatorStyles[elemId] = parseMouseStyle(styleObj);
                LOG_DEBUG(COMP, QString("  🖱️  Mouse indicator style for element %1").arg(elemId));
            } else {
                style.keyElementStyles[elemId] = parseKeyStyle(styleObj);
                LOG_DEBUG(COMP, QString("  🎨 Key style for element %1").arg(elemId));
            }
        }
    } else if (elemStylesVal.isObject()) {
        QJsonObject elemStyles = elemStylesVal.toObject();
        for (auto it = elemStyles.begin(); it != elemStyles.end(); ++it) {
            int elemId = it.key().toInt();
            QJsonObject styleObj = it.value().toObject();

            if (styleObj.contains("InnerColor") || styleObj.contains("OuterColor")) {
                style.mouseIndicatorStyles[elemId] = parseMouseStyle(styleObj);
                LOG_DEBUG(COMP, QString("  🖱️  Mouse indicator style for element %1").arg(elemId));
            } else {
                style.keyElementStyles[elemId] = parseKeyStyle(styleObj);
                LOG_DEBUG(COMP, QString("  🎨 Key style for element %1").arg(elemId));
            }
        }
    }

    LOG_INFO(COMP, QString("✅ Style loaded — %1 element overrides")
                       .arg(style.keyElementStyles.size() + style.mouseIndicatorStyles.size()));
    return style;
}

KeyboardStyle StyleLoader::defaultStyle() {
    KeyboardStyle s;
    s.backgroundColor = QColor(30, 30, 30);

    // Default loose key
    s.defaultKeyStyle.loose.background = QColor(70, 70, 70);
    s.defaultKeyStyle.loose.text = QColor(220, 220, 220);
    s.defaultKeyStyle.loose.outline = QColor(120, 120, 120);
    s.defaultKeyStyle.loose.showOutline = true;
    s.defaultKeyStyle.loose.outlineWidth = 1;
    s.defaultKeyStyle.loose.font.fontFamily = "Arial";
    s.defaultKeyStyle.loose.font.size = 9.0f;

    // Default pressed key
    s.defaultKeyStyle.pressed.background = QColor(180, 100, 0);
    s.defaultKeyStyle.pressed.text = QColor(255, 255, 255);
    s.defaultKeyStyle.pressed.outline = QColor(255, 160, 0);
    s.defaultKeyStyle.pressed.showOutline = true;
    s.defaultKeyStyle.pressed.outlineWidth = 2;
    s.defaultKeyStyle.pressed.font = s.defaultKeyStyle.loose.font;

    // Mouse indicator
    s.defaultMouseIndicatorStyle.outerColor = QColor(200, 200, 200);
    s.defaultMouseIndicatorStyle.innerColor = QColor(0, 200, 0);
    s.defaultMouseIndicatorStyle.outlineWidth = 3;

    return s;
}

QColor StyleLoader::parseColor(const QJsonObject &obj) {
    int r = obj.value("Red").toInt(128);
    int g = obj.value("Green").toInt(128);
    int b = obj.value("Blue").toInt(128);
    return QColor(r, g, b);
}

NohFont StyleLoader::parseFont(const QJsonObject &obj) {
    NohFont f;
    f.fontFamily = obj.value("FontFamily").toString("Arial");
    f.size = static_cast<float>(obj.value("Size").toDouble(10.0));
    f.style = obj.value("Style").toInt(0);
    return f;
}

KeySubStyle StyleLoader::parseKeySubStyle(const QJsonObject &obj) {
    KeySubStyle s;
    if (obj.contains("Background"))
        s.background = parseColor(obj.value("Background").toObject());
    if (obj.contains("Text"))
        s.text = parseColor(obj.value("Text").toObject());
    if (obj.contains("Outline"))
        s.outline = parseColor(obj.value("Outline").toObject());
    s.showOutline = obj.value("ShowOutline").toBool(true);
    s.outlineWidth = obj.value("OutlineWidth").toInt(1);
    if (obj.contains("Font"))
        s.font = parseFont(obj.value("Font").toObject());

    // Clean up background image filename
    QString bgImg = obj.value("BackgroundImageFileName").toString().trimmed();
    if (bgImg.isEmpty() || bgImg == "null" || bgImg == "NULL") {
        s.backgroundImageFileName = QString();
    } else {
        s.backgroundImageFileName = bgImg;
    }

    return s;
}

KeyStyle StyleLoader::parseKeyStyle(const QJsonObject &obj) {
    KeyStyle s;
    if (obj.contains("Loose"))
        s.loose = parseKeySubStyle(obj.value("Loose").toObject());
    if (obj.contains("Pressed"))
        s.pressed = parseKeySubStyle(obj.value("Pressed").toObject());
    return s;
}

MouseSpeedIndicatorStyle StyleLoader::parseMouseStyle(const QJsonObject &obj) {
    MouseSpeedIndicatorStyle s;
    if (obj.contains("InnerColor"))
        s.innerColor = parseColor(obj.value("InnerColor").toObject());
    if (obj.contains("OuterColor"))
        s.outerColor = parseColor(obj.value("OuterColor").toObject());
    s.outlineWidth = obj.value("OutlineWidth").toInt(3);
    return s;
}

bool StyleLoader::save(const KeyboardStyle &style, const QString &filePath) {
    QJsonObject root;

    // Background color
    QJsonObject bgColorObj;
    bgColorObj["Red"] = style.backgroundColor.red();
    bgColorObj["Green"] = style.backgroundColor.green();
    bgColorObj["Blue"] = style.backgroundColor.blue();
    root["BackgroundColor"] = bgColorObj;
    root["BackgroundImageFileName"] = style.backgroundImageFileName;

    // DefaultKeyStyle
    QJsonObject defaultKeyStyleObj;
    // Loose
    QJsonObject looseObj;
    looseObj["Background"] = colorToJson(style.defaultKeyStyle.loose.background);
    looseObj["BackgroundImageFileName"] = style.defaultKeyStyle.loose.backgroundImageFileName;
    looseObj["Font"] = fontToJson(style.defaultKeyStyle.loose.font);
    looseObj["Outline"] = colorToJson(style.defaultKeyStyle.loose.outline);
    looseObj["OutlineWidth"] = style.defaultKeyStyle.loose.outlineWidth;
    looseObj["ShowOutline"] = style.defaultKeyStyle.loose.showOutline;
    looseObj["Text"] = colorToJson(style.defaultKeyStyle.loose.text);
    defaultKeyStyleObj["Loose"] = looseObj;

    // Pressed
    QJsonObject pressedObj;
    pressedObj["Background"] = colorToJson(style.defaultKeyStyle.pressed.background);
    pressedObj["BackgroundImageFileName"] = style.defaultKeyStyle.pressed.backgroundImageFileName;
    pressedObj["Font"] = fontToJson(style.defaultKeyStyle.pressed.font);
    pressedObj["Outline"] = colorToJson(style.defaultKeyStyle.pressed.outline);
    pressedObj["OutlineWidth"] = style.defaultKeyStyle.pressed.outlineWidth;
    pressedObj["ShowOutline"] = style.defaultKeyStyle.pressed.showOutline;
    pressedObj["Text"] = colorToJson(style.defaultKeyStyle.pressed.text);
    defaultKeyStyleObj["Pressed"] = pressedObj;
    root["DefaultKeyStyle"] = defaultKeyStyleObj;

    // DefaultMouseSpeedIndicatorStyle
    QJsonObject mouseIndicatorObj;
    mouseIndicatorObj["InnerColor"] = colorToJson(style.defaultMouseIndicatorStyle.innerColor);
    mouseIndicatorObj["OuterColor"] = colorToJson(style.defaultMouseIndicatorStyle.outerColor);
    mouseIndicatorObj["OutlineWidth"] = style.defaultMouseIndicatorStyle.outlineWidth;
    root["DefaultMouseSpeedIndicatorStyle"] = mouseIndicatorObj;

    // ElementStyles
    QJsonArray elemStylesArray;
    // Keyboard key styles
    for (auto it = style.keyElementStyles.begin(); it != style.keyElementStyles.end(); ++it) {
        QJsonObject item;
        item["Key"] = it.key();
        QJsonObject valueObj;
        valueObj["__type"] = "KeyStyle";
        // Loose
        QJsonObject looseElem;
        looseElem["Background"] = colorToJson(it.value().loose.background);
        looseElem["BackgroundImageFileName"] = it.value().loose.backgroundImageFileName;
        looseElem["Font"] = fontToJson(it.value().loose.font);
        looseElem["Outline"] = colorToJson(it.value().loose.outline);
        looseElem["OutlineWidth"] = it.value().loose.outlineWidth;
        looseElem["ShowOutline"] = it.value().loose.showOutline;
        looseElem["Text"] = colorToJson(it.value().loose.text);
        valueObj["Loose"] = looseElem;
        // Pressed
        QJsonObject pressedElem;
        pressedElem["Background"] = colorToJson(it.value().pressed.background);
        pressedElem["BackgroundImageFileName"] = it.value().pressed.backgroundImageFileName;
        pressedElem["Font"] = fontToJson(it.value().pressed.font);
        pressedElem["Outline"] = colorToJson(it.value().pressed.outline);
        pressedElem["OutlineWidth"] = it.value().pressed.outlineWidth;
        pressedElem["ShowOutline"] = it.value().pressed.showOutline;
        pressedElem["Text"] = colorToJson(it.value().pressed.text);
        valueObj["Pressed"] = pressedElem;

        item["Value"] = valueObj;
        elemStylesArray.append(item);
    }
    // Mouse indicator styles
    for (auto it = style.mouseIndicatorStyles.begin(); it != style.mouseIndicatorStyles.end(); ++it) {
        QJsonObject item;
        item["Key"] = it.key();
        QJsonObject valueObj;
        valueObj["__type"] = "MouseSpeedIndicatorStyle"; // or whatever type
        valueObj["InnerColor"] = colorToJson(it.value().innerColor);
        valueObj["OuterColor"] = colorToJson(it.value().outerColor);
        valueObj["OutlineWidth"] = it.value().outlineWidth;
        item["Value"] = valueObj;
        elemStylesArray.append(item);
    }
    if (!elemStylesArray.isEmpty())
        root["ElementStyles"] = elemStylesArray;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(QJsonDocument(root).toJson());
    file.close();
    return true;
}