#pragma once
#include <QDialog>
#include "../models/keyboardstyle.h"

class StyleDialog : public QDialog {
    Q_OBJECT
public:
    explicit StyleDialog(const KeyboardStyle& style, QWidget* parent = nullptr);
    KeyboardStyle style() const { return m_style; }

private:
    KeyboardStyle m_style;
};
