#include "definitionloader.h"
#include "../logger.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

static const QString COMP = "DefinitionLoader";

std::optional<KeyboardDefinition> DefinitionLoader::load(const QString &filePath) {
    LOG_INFO(COMP, QString("📂 Loading keyboard definition: %1").arg(filePath));

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR(COMP, QString("❌ Cannot open file: %1 — %2").arg(filePath, file.errorString()));
        return std::nullopt;
    }

    QByteArray data = file.readAll();
    file.close();
    LOG_DEBUG(COMP, QString("📄 Read %1 bytes from file").arg(data.size()));

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        LOG_ERROR(COMP, QString("❌ JSON parse error at offset %1: %2")
                            .arg(parseError.offset)
                            .arg(parseError.errorString()));
        return std::nullopt;
    }

    if (!doc.isObject()) {
        LOG_ERROR(COMP, "❌ Root JSON value is not an object");
        return std::nullopt;
    }

    QJsonObject root = doc.object();
    KeyboardDefinition def;

    def.version = root.value("Version").toInt(2);
    def.width = root.value("Width").toInt(800);
    def.height = root.value("Height").toInt(300);

    LOG_INFO(COMP, QString("📐 Keyboard size: %1 x %2  (version %3)")
                       .arg(def.width)
                       .arg(def.height)
                       .arg(def.version));

    QJsonArray elements = root.value("Elements").toArray();
    LOG_INFO(COMP, QString("🔑 Found %1 elements").arg(elements.size()));

    QSet<int> seenIds;
    int parsed = 0, skipped = 0;

    for (const QJsonValue &val : elements) {
        if (!val.isObject()) {
            LOG_WARN(COMP, "⚠️  Element is not a JSON object, skipping");
            skipped++;
            continue;
        }

        auto elem = parseElement(val.toObject());
        if (!elem) {
            LOG_WARN(COMP, "⚠️  Failed to parse element, skipping");
            skipped++;
            continue;
        }

        if (seenIds.contains(elem->id)) {
            LOG_WARN(COMP, QString("⚠️  Duplicate element ID %1, skipping").arg(elem->id));
            skipped++;
            continue;
        }

        seenIds.insert(elem->id);
        def.elements.append(elem);
        parsed++;
    }

    LOG_INFO(COMP, QString("✅ Parsed %1 elements (%2 skipped)").arg(parsed).arg(skipped));
    return def;
}

TPoint DefinitionLoader::parsePoint(const QJsonObject &obj) {
    TPoint p;
    p.x = obj.value("X").toInt(0);
    p.y = obj.value("Y").toInt(0);
    return p;
}

std::shared_ptr<ElementDefinition> DefinitionLoader::parseElement(const QJsonObject &obj) {
    // Detect type by presence of fields
    bool hasBoundaries = obj.contains("Boundaries");
    bool hasLocation = obj.contains("Location");
    bool hasRadius = obj.contains("Radius");
    bool hasShiftText = obj.contains("ShiftText");
    bool hasChangeOnCaps = obj.contains("ChangeOnCaps");

    int id = obj.value("Id").toInt(-1);
    if (id < 0) {
        LOG_WARN(COMP, "⚠️  Element has no valid Id");
    }

    // ── MouseSpeedIndicator ──────────────────────────────────────────────────
    if (hasLocation && hasRadius) {
        auto elem = std::make_shared<MouseSpeedIndicatorDefinition>();
        elem->id = id;
        elem->radius = obj.value("Radius").toInt(25);
        elem->location = parsePoint(obj.value("Location").toObject());
        elem->textPosition = elem->location;
        LOG_DEBUG(COMP, QString("  🖱️  MouseSpeedIndicator id=%1 radius=%2").arg(id).arg(elem->radius));
        return elem;
    }

    // Parse common fields
    QVector<TPoint> boundaries;
    QJsonArray bArr = obj.value("Boundaries").toArray();
    for (const QJsonValue &bv : bArr) {
        boundaries.append(parsePoint(bv.toObject()));
    }

    TPoint textPos = parsePoint(obj.value("TextPosition").toObject());

    QVector<int> keyCodes;
    QJsonArray kcArr = obj.value("KeyCodes").toArray();
    for (const QJsonValue &kv : kcArr) {
        keyCodes.append(kv.toInt());
    }

    QString text = obj.value("Text").toString();

    // ── KeyboardKey ──────────────────────────────────────────────────────────
    if (hasShiftText || hasChangeOnCaps) {
        auto elem = std::make_shared<KeyboardKeyDefinition>();
        elem->id = id;
        elem->boundaries = boundaries;
        elem->textPosition = textPos;
        elem->keyCodes = keyCodes;
        elem->text = text;
        elem->shiftText = obj.value("ShiftText").toString(text);
        elem->changeOnCaps = obj.value("ChangeOnCaps").toBool(true);
        LOG_DEBUG(COMP, QString("  ⌨️  KeyboardKey id=%1 text='%2' shiftText='%3' changeOnCaps=%4 codes=%5")
                            .arg(id)
                            .arg(text)
                            .arg(obj.value("ShiftText").toString())
                            .arg(obj.value("ChangeOnCaps").toBool())
                            .arg(keyCodes.size()));
        return elem;
    }

    // Discriminate MouseKey vs MouseScroll by key code range convention
    // (MouseScroll keycodes are typically 0-3 scroll directions when parsed from scroll context)
    // We detect by __type field if present, else fall back to MouseKey
    QString typeName = obj.value("__type").toString();

    if (typeName == "MouseScrollDefinition" || typeName == "MouseScroll") {
        auto elem = std::make_shared<MouseScrollDefinition>();
        elem->id = id;
        elem->boundaries = boundaries;
        elem->textPosition = textPos;
        elem->keyCodes = keyCodes;
        elem->text = text;
        LOG_DEBUG(COMP, QString("  🖱️  MouseScroll id=%1 text='%2'").arg(id).arg(text));
        return elem;
    }

    // Default: MouseKey
    auto elem = std::make_shared<MouseKeyDefinition>();
    elem->id = id;
    elem->boundaries = boundaries;
    elem->textPosition = textPos;
    elem->keyCodes = keyCodes;
    elem->text = text;
    LOG_DEBUG(COMP, QString("  🖱️  MouseKey id=%1 text='%2'").arg(id).arg(text));
    return elem;
}
