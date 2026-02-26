#pragma once
#include "models/keyboardstyle.h"
#include <QCheckBox>
#include <QDialog>
#include <QFontComboBox>
#include <QLineEdit>
#include <QSpinBox>

class KeyStyleDialog : public QDialog {
    Q_OBJECT
  public:
    explicit KeyStyleDialog(const KeyStyle &style, bool isOverride, QWidget *parent = nullptr);
    ~KeyStyleDialog() override;
    KeyStyle style() const { return m_style; }
    bool overwriteDefault() const { return m_overwriteCheck->isChecked(); }

  private slots:
    void onLooseColorClicked();
    void onPressedColorClicked();
    void onLooseOutlineColorClicked();
    void onPressedOutlineColorClicked();
    void onLooseTextColorClicked();
    void onPressedTextColorClicked();
    void accept() override;
    void onOverwriteToggled(bool checked);

  private:
    KeyStyle m_style;
    bool m_isOverride;
    QCheckBox *m_overwriteCheck;

    // Loose style
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

    // Pressed style
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