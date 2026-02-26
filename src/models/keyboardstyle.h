#pragma once
#include <QString>
#include <QColor>
#include <QFont>
#include <QMap>
#include <optional>

// ─── Font Style ──────────────────────────────────────────────────────────────

struct NohFont {
    QString fontFamily = "Arial";
    float size = 10.0f;
    int style = 0; // 0=Regular,1=Bold,2=Italic,4=Underline,8=Strikeout

    QFont toQFont() const {
        QFont f(fontFamily, static_cast<int>(size));
        f.setBold(style & 1);
        f.setItalic(style & 2);
        f.setUnderline(style & 4);
        f.setStrikeOut(style & 8);
        return f;
    }
};

// ─── Key Sub Style ───────────────────────────────────────────────────────────

struct KeySubStyle {
    QColor background = QColor(100, 100, 100);
    QColor text = QColor(255, 255, 255);
    QColor outline = QColor(0, 0, 0);
    bool showOutline = true;
    int outlineWidth = 1;
    NohFont font;
    QString backgroundImageFileName;
};

// ─── Key Style ───────────────────────────────────────────────────────────────

struct KeyStyle {
    KeySubStyle loose;
    KeySubStyle pressed;
};

// ─── Mouse Speed Indicator Style ─────────────────────────────────────────────

struct MouseSpeedIndicatorStyle {
    QColor innerColor = QColor(0, 255, 0);
    QColor outerColor = QColor(255, 0, 0);
    int outlineWidth = 3;
};

// ─── Keyboard Style ──────────────────────────────────────────────────────────

struct KeyboardStyle {
    QColor backgroundColor = QColor(0, 0, 0);
    QString backgroundImageFileName;
    KeyStyle defaultKeyStyle;
    MouseSpeedIndicatorStyle defaultMouseIndicatorStyle;
    QMap<int, KeyStyle> keyElementStyles;
    QMap<int, MouseSpeedIndicatorStyle> mouseIndicatorStyles;

    bool isEmpty() const { return backgroundColor == QColor(0,0,0) && backgroundImageFileName.isEmpty(); }
};
