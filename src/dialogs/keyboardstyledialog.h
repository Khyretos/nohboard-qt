#pragma once
#include "models/keyboardstyle.h"
#include <QCheckBox>
#include <QDialog>
#include <QFontComboBox>
#include <QLineEdit>
#include <QSpinBox>

class KeyboardStyleDialog : public QDialog {
    Q_OBJECT
  public:
    explicit KeyboardStyleDialog(const KeyboardStyle &style, QWidget *parent = nullptr);
    ~KeyboardStyleDialog() override;
    KeyboardStyle style() const { return m_style; }

  private slots:
    void onBackgroundColorClicked();
    void onLooseColorClicked();
    void onPressedColorClicked();
    void onMouseInnerColorClicked();
    void onMouseOuterColorClicked();
    void onLooseOutlineColorClicked();
    void onPressedOutlineColorClicked();
    void onLooseTextColorClicked();
    void onPressedTextColorClicked();
    void accept() override;

  private:
    KeyboardStyle m_style;

    // Background
    QLineEdit *m_bgColorEdit;
    QLineEdit *m_bgImageEdit;

    // Mouse indicator
    QLineEdit *m_mouseInnerColorEdit;
    QLineEdit *m_mouseOuterColorEdit;
    QSpinBox *m_mouseOutlineWidthSpin;

    // Loose key style
    QLineEdit *m_looseColorEdit;
    QLineEdit *m_looseImageEdit;
    QFontComboBox *m_looseFontCombo;
    QSpinBox *m_looseFontSizeSpin;
    QCheckBox *m_looseBoldCheck;
    QCheckBox *m_looseItalicCheck;
    QLineEdit *m_looseOutlineColorEdit;
    QCheckBox *m_looseShowOutlineCheck;
    QSpinBox *m_looseOutlineWidthSpin;
    QLineEdit *m_looseTextColorEdit;

    // Pressed key style
    QLineEdit *m_pressedColorEdit;
    QLineEdit *m_pressedImageEdit;
    QFontComboBox *m_pressedFontCombo;
    QSpinBox *m_pressedFontSizeSpin;
    QCheckBox *m_pressedBoldCheck;
    QCheckBox *m_pressedItalicCheck;
    QLineEdit *m_pressedOutlineColorEdit;
    QCheckBox *m_pressedShowOutlineCheck;
    QSpinBox *m_pressedOutlineWidthSpin;
    QLineEdit *m_pressedTextColorEdit;

    void setupUi();
    void updateFromStyle();
    QColor pickColor(const QColor &initial);
};