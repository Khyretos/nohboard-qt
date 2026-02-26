#pragma once
#include "../models/keyboarddefinition.h"
#include <QString>
#include <optional>

class DefinitionLoader {
public:
    static std::optional<KeyboardDefinition> load(const QString& filePath);

private:
    static TPoint parsePoint(const QJsonObject& obj);
    static std::shared_ptr<ElementDefinition> parseElement(const QJsonObject& obj);
};
