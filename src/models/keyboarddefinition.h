#pragma once
#include <QString>
#include <QPoint>
#include <QVector>
#include <QVariant>
#include <memory>

// ─── Element Types ──────────────────────────────────────────────────────────

enum class ElementType {
    KeyboardKey,
    MouseKey,
    MouseScroll,
    MouseSpeedIndicator
};

struct TPoint {
    int x = 0;
    int y = 0;
    QPoint toQPoint() const { return QPoint(x, y); }
};

// ─── Base Element ────────────────────────────────────────────────────────────

struct ElementDefinition {
    int id = 0;
    ElementType type = ElementType::KeyboardKey;
    QVector<TPoint> boundaries;
    TPoint textPosition;
    QVector<int> keyCodes;
    QString text;

    virtual ~ElementDefinition() = default;
    virtual ElementType elementType() const { return type; }
};

// ─── Keyboard Key ────────────────────────────────────────────────────────────

struct KeyboardKeyDefinition : ElementDefinition {
    QString shiftText;
    bool changeOnCaps = true;

    KeyboardKeyDefinition() { type = ElementType::KeyboardKey; }
};

// ─── Mouse Key ───────────────────────────────────────────────────────────────

struct MouseKeyDefinition : ElementDefinition {
    MouseKeyDefinition() { type = ElementType::MouseKey; }
};

// ─── Mouse Scroll ────────────────────────────────────────────────────────────

struct MouseScrollDefinition : ElementDefinition {
    MouseScrollDefinition() { type = ElementType::MouseScroll; }
};

// ─── Mouse Speed Indicator ───────────────────────────────────────────────────

struct MouseSpeedIndicatorDefinition : ElementDefinition {
    TPoint location;
    int radius = 25;

    MouseSpeedIndicatorDefinition() { type = ElementType::MouseSpeedIndicator; }
};

// ─── Keyboard Definition ─────────────────────────────────────────────────────

struct KeyboardDefinition {
    int version = 2;
    int width = 800;
    int height = 300;
    QVector<std::shared_ptr<ElementDefinition>> elements;

    // Helpers
    bool isEmpty() const { return elements.isEmpty(); }
};

// ─── Mouse Key Codes ─────────────────────────────────────────────────────────
namespace MouseKeyCodes {
    constexpr int Left   = 0;
    constexpr int Right  = 1;
    constexpr int Middle = 2;
    constexpr int X1     = 3;
    constexpr int X2     = 4;
}

// ─── Scroll Key Codes ────────────────────────────────────────────────────────
namespace ScrollKeyCodes {
    constexpr int Up    = 0;
    constexpr int Down  = 1;
    constexpr int Right = 2;
    constexpr int Left  = 3;
}

// ─── Special Key Codes ───────────────────────────────────────────────────────
namespace SpecialKeyCodes {
    constexpr int SecondEnter  = 1025;
    constexpr int CapsLockState = 1026;
    constexpr int NumLockState  = 1027;
    constexpr int ScrollLockState = 1028;
}
