#pragma once
#include "../models/keyboardstyle.h"
#include <QJsonObject>
#include <QString>
#include <optional>

class StyleLoader {
  public:
    static std::optional<KeyboardStyle> load(const QString &filePath);
    static KeyboardStyle defaultStyle();
    static bool save(const KeyboardStyle &style, const QString &filePath);

  private:
    static QColor parseColor(const QJsonObject &obj);
    static NohFont parseFont(const QJsonObject &obj);
    static KeySubStyle parseKeySubStyle(const QJsonObject &obj);
    static KeyStyle parseKeyStyle(const QJsonObject &obj);
    static MouseSpeedIndicatorStyle parseMouseStyle(const QJsonObject &obj);
};
