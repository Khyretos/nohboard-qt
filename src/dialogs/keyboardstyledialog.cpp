#include "keyboardstyledialog.h"
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

KeyboardStyleDialog::KeyboardStyleDialog(const KeyboardStyle &style, QWidget *parent)
    : QDialog(parent), m_style(style) {
    setWindowTitle("Keyboard Style");
    setupUi();
    updateFromStyle();
}

KeyboardStyleDialog::~KeyboardStyleDialog() {}

void KeyboardStyleDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- Background group ---
    QGroupBox *bgGroup = new QGroupBox("Keyboard");
    QFormLayout *bgLayout = new QFormLayout(bgGroup);

    m_bgColorEdit = new QLineEdit();
    m_bgColorEdit->setReadOnly(true);
    QPushButton *bgColorBtn = new QPushButton("Choose...");
    connect(bgColorBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onBackgroundColorClicked);
    QHBoxLayout *bgColorLayout = new QHBoxLayout();
    bgColorLayout->addWidget(m_bgColorEdit);
    bgColorLayout->addWidget(bgColorBtn);
    bgLayout->addRow("Background Color:", bgColorLayout);

    m_bgImageEdit = new QLineEdit();
    bgLayout->addRow("Background Image:", m_bgImageEdit);
    mainLayout->addWidget(bgGroup);

    // --- Mouse indicator group ---
    QGroupBox *mouseGroup = new QGroupBox("MouseSpeedIndicator");
    QFormLayout *mouseLayout = new QFormLayout(mouseGroup);

    m_mouseInnerColorEdit = new QLineEdit();
    m_mouseInnerColorEdit->setReadOnly(true);
    QPushButton *innerColorBtn = new QPushButton("Choose...");
    connect(innerColorBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onMouseInnerColorClicked);
    QHBoxLayout *innerColorLayout = new QHBoxLayout();
    innerColorLayout->addWidget(m_mouseInnerColorEdit);
    innerColorLayout->addWidget(innerColorBtn);
    mouseLayout->addRow("Inner Color:", innerColorLayout);

    m_mouseOuterColorEdit = new QLineEdit();
    m_mouseOuterColorEdit->setReadOnly(true);
    QPushButton *outerColorBtn = new QPushButton("Choose...");
    connect(outerColorBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onMouseOuterColorClicked);
    QHBoxLayout *outerColorLayout = new QHBoxLayout();
    outerColorLayout->addWidget(m_mouseOuterColorEdit);
    outerColorLayout->addWidget(outerColorBtn);
    mouseLayout->addRow("Outer Color:", outerColorLayout);

    m_mouseOutlineWidthSpin = new QSpinBox();
    m_mouseOutlineWidthSpin->setRange(0, 10);
    mouseLayout->addRow("Outline Width:", m_mouseOutlineWidthSpin);
    mainLayout->addWidget(mouseGroup);

    // --- Loose key style group ---
    QGroupBox *looseGroup = new QGroupBox("Loose Keys");
    QFormLayout *looseLayout = new QFormLayout(looseGroup);

    m_looseColorEdit = new QLineEdit();
    m_looseColorEdit->setReadOnly(true);
    QPushButton *looseColorBtn = new QPushButton("Choose...");
    connect(looseColorBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onLooseColorClicked);
    QHBoxLayout *looseColorLayout = new QHBoxLayout();
    looseColorLayout->addWidget(m_looseColorEdit);
    looseColorLayout->addWidget(looseColorBtn);
    looseLayout->addRow("Background Color:", looseColorLayout);

    m_looseImageEdit = new QLineEdit();
    looseLayout->addRow("Image:", m_looseImageEdit);

    m_looseFontCombo = new QFontComboBox();
    looseLayout->addRow("Font:", m_looseFontCombo);

    m_looseFontSizeSpin = new QSpinBox();
    m_looseFontSizeSpin->setRange(1, 72);
    looseLayout->addRow("Font Size:", m_looseFontSizeSpin);

    m_looseBoldCheck = new QCheckBox("Bold");
    m_looseItalicCheck = new QCheckBox("Italic");
    QHBoxLayout *fontStyleLayout = new QHBoxLayout();
    fontStyleLayout->addWidget(m_looseBoldCheck);
    fontStyleLayout->addWidget(m_looseItalicCheck);
    looseLayout->addRow("Font Style:", fontStyleLayout);

    m_looseOutlineColorEdit = new QLineEdit();
    m_looseOutlineColorEdit->setReadOnly(true);
    QPushButton *looseOutlineBtn = new QPushButton("Choose...");
    connect(looseOutlineBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onLooseOutlineColorClicked);
    QHBoxLayout *looseOutlineLayout = new QHBoxLayout();
    looseOutlineLayout->addWidget(m_looseOutlineColorEdit);
    looseOutlineLayout->addWidget(looseOutlineBtn);
    looseLayout->addRow("Outline Color:", looseOutlineLayout);

    m_looseShowOutlineCheck = new QCheckBox("Show Outline");
    looseLayout->addRow("", m_looseShowOutlineCheck);

    m_looseOutlineWidthSpin = new QSpinBox();
    m_looseOutlineWidthSpin->setRange(0, 10);
    looseLayout->addRow("Outline Width:", m_looseOutlineWidthSpin);

    m_looseTextColorEdit = new QLineEdit();
    m_looseTextColorEdit->setReadOnly(true);
    QPushButton *looseTextColorBtn = new QPushButton("Choose...");
    connect(looseTextColorBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onLooseTextColorClicked);
    QHBoxLayout *looseTextLayout = new QHBoxLayout();
    looseTextLayout->addWidget(m_looseTextColorEdit);
    looseTextLayout->addWidget(looseTextColorBtn);
    looseLayout->addRow("Text Color:", looseTextLayout);

    mainLayout->addWidget(looseGroup);

    // --- Pressed key style group (similar) ---
    QGroupBox *pressedGroup = new QGroupBox("Pressed Keys");
    QFormLayout *pressedLayout = new QFormLayout(pressedGroup);

    m_pressedColorEdit = new QLineEdit();
    m_pressedColorEdit->setReadOnly(true);
    QPushButton *pressedColorBtn = new QPushButton("Choose...");
    connect(pressedColorBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onPressedColorClicked);
    QHBoxLayout *pressedColorLayout = new QHBoxLayout();
    pressedColorLayout->addWidget(m_pressedColorEdit);
    pressedColorLayout->addWidget(pressedColorBtn);
    pressedLayout->addRow("Background Color:", pressedColorLayout);

    m_pressedImageEdit = new QLineEdit();
    pressedLayout->addRow("Image:", m_pressedImageEdit);

    m_pressedFontCombo = new QFontComboBox();
    pressedLayout->addRow("Font:", m_pressedFontCombo);

    m_pressedFontSizeSpin = new QSpinBox();
    m_pressedFontSizeSpin->setRange(1, 72);
    pressedLayout->addRow("Font Size:", m_pressedFontSizeSpin);

    m_pressedBoldCheck = new QCheckBox("Bold");
    m_pressedItalicCheck = new QCheckBox("Italic");
    QHBoxLayout *pressedFontStyleLayout = new QHBoxLayout();
    pressedFontStyleLayout->addWidget(m_pressedBoldCheck);
    pressedFontStyleLayout->addWidget(m_pressedItalicCheck);
    pressedLayout->addRow("Font Style:", pressedFontStyleLayout);

    m_pressedOutlineColorEdit = new QLineEdit();
    m_pressedOutlineColorEdit->setReadOnly(true);
    QPushButton *pressedOutlineBtn = new QPushButton("Choose...");
    connect(pressedOutlineBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onPressedOutlineColorClicked);
    QHBoxLayout *pressedOutlineLayout = new QHBoxLayout();
    pressedOutlineLayout->addWidget(m_pressedOutlineColorEdit);
    pressedOutlineLayout->addWidget(pressedOutlineBtn);
    pressedLayout->addRow("Outline Color:", pressedOutlineLayout);

    m_pressedShowOutlineCheck = new QCheckBox("Show Outline");
    pressedLayout->addRow("", m_pressedShowOutlineCheck);

    m_pressedOutlineWidthSpin = new QSpinBox();
    m_pressedOutlineWidthSpin->setRange(0, 10);
    pressedLayout->addRow("Outline Width:", m_pressedOutlineWidthSpin);

    m_pressedTextColorEdit = new QLineEdit();
    m_pressedTextColorEdit->setReadOnly(true);
    QPushButton *pressedTextColorBtn = new QPushButton("Choose...");
    connect(pressedTextColorBtn, &QPushButton::clicked, this, &KeyboardStyleDialog::onPressedTextColorClicked);
    QHBoxLayout *pressedTextLayout = new QHBoxLayout();
    pressedTextLayout->addWidget(m_pressedTextColorEdit);
    pressedTextLayout->addWidget(pressedTextColorBtn);
    pressedLayout->addRow("Text Color:", pressedTextLayout);

    mainLayout->addWidget(pressedGroup);

    // --- Dialog buttons ---
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void KeyboardStyleDialog::updateFromStyle() {
    m_bgColorEdit->setText(m_style.backgroundColor.name());
    m_bgImageEdit->setText(m_style.backgroundImageFileName);

    m_mouseInnerColorEdit->setText(m_style.defaultMouseIndicatorStyle.innerColor.name());
    m_mouseOuterColorEdit->setText(m_style.defaultMouseIndicatorStyle.outerColor.name());
    m_mouseOutlineWidthSpin->setValue(m_style.defaultMouseIndicatorStyle.outlineWidth);

    // Loose
    const auto &loose = m_style.defaultKeyStyle.loose;
    m_looseColorEdit->setText(loose.background.name());
    m_looseImageEdit->setText(loose.backgroundImageFileName);
    m_looseFontCombo->setCurrentFont(QFont(loose.font.fontFamily));
    m_looseFontSizeSpin->setValue(static_cast<int>(loose.font.size));
    m_looseBoldCheck->setChecked(loose.font.style & 1);
    m_looseItalicCheck->setChecked(loose.font.style & 2);
    m_looseOutlineColorEdit->setText(loose.outline.name());
    m_looseShowOutlineCheck->setChecked(loose.showOutline);
    m_looseOutlineWidthSpin->setValue(loose.outlineWidth);
    m_looseTextColorEdit->setText(loose.text.name());

    // Pressed
    const auto &pressed = m_style.defaultKeyStyle.pressed;
    m_pressedColorEdit->setText(pressed.background.name());
    m_pressedImageEdit->setText(pressed.backgroundImageFileName);
    m_pressedFontCombo->setCurrentFont(QFont(pressed.font.fontFamily));
    m_pressedFontSizeSpin->setValue(static_cast<int>(pressed.font.size));
    m_pressedBoldCheck->setChecked(pressed.font.style & 1);
    m_pressedItalicCheck->setChecked(pressed.font.style & 2);
    m_pressedOutlineColorEdit->setText(pressed.outline.name());
    m_pressedShowOutlineCheck->setChecked(pressed.showOutline);
    m_pressedOutlineWidthSpin->setValue(pressed.outlineWidth);
    m_pressedTextColorEdit->setText(pressed.text.name());
}

QColor KeyboardStyleDialog::pickColor(const QColor &initial) {
    return QColorDialog::getColor(initial, this, "Choose Color");
}

void KeyboardStyleDialog::accept() {
    // Update m_style from UI
    m_style.backgroundColor = QColor(m_bgColorEdit->text());
    m_style.backgroundImageFileName = m_bgImageEdit->text();

    m_style.defaultMouseIndicatorStyle.innerColor = QColor(m_mouseInnerColorEdit->text());
    m_style.defaultMouseIndicatorStyle.outerColor = QColor(m_mouseOuterColorEdit->text());
    m_style.defaultMouseIndicatorStyle.outlineWidth = m_mouseOutlineWidthSpin->value();

    auto &loose = m_style.defaultKeyStyle.loose;
    loose.background = QColor(m_looseColorEdit->text());
    loose.backgroundImageFileName = m_looseImageEdit->text();
    loose.font.fontFamily = m_looseFontCombo->currentFont().family();
    loose.font.size = static_cast<float>(m_looseFontSizeSpin->value());
    int style = 0;
    if (m_looseBoldCheck->isChecked()) style |= 1;
    if (m_looseItalicCheck->isChecked()) style |= 2;
    loose.font.style = style;
    loose.outline = QColor(m_looseOutlineColorEdit->text());
    loose.showOutline = m_looseShowOutlineCheck->isChecked();
    loose.outlineWidth = m_looseOutlineWidthSpin->value();
    loose.text = QColor(m_looseTextColorEdit->text());

    auto &pressed = m_style.defaultKeyStyle.pressed;
    pressed.background = QColor(m_pressedColorEdit->text());
    pressed.backgroundImageFileName = m_pressedImageEdit->text();
    pressed.font.fontFamily = m_pressedFontCombo->currentFont().family();
    pressed.font.size = static_cast<float>(m_pressedFontSizeSpin->value());
    style = 0;
    if (m_pressedBoldCheck->isChecked()) style |= 1;
    if (m_pressedItalicCheck->isChecked()) style |= 2;
    pressed.font.style = style;
    pressed.outline = QColor(m_pressedOutlineColorEdit->text());
    pressed.showOutline = m_pressedShowOutlineCheck->isChecked();
    pressed.outlineWidth = m_pressedOutlineWidthSpin->value();
    pressed.text = QColor(m_pressedTextColorEdit->text());

    QDialog::accept();
}

void KeyboardStyleDialog::onBackgroundColorClicked() {
    QColor c = pickColor(m_style.backgroundColor);
    if (c.isValid()) {
        m_style.backgroundColor = c;
        m_bgColorEdit->setText(c.name());
    }
}

void KeyboardStyleDialog::onLooseColorClicked() {
    QColor c = pickColor(m_style.defaultKeyStyle.loose.background);
    if (c.isValid()) {
        m_style.defaultKeyStyle.loose.background = c;
        m_looseColorEdit->setText(c.name());
    }
}

void KeyboardStyleDialog::onPressedColorClicked() {
    QColor c = pickColor(m_style.defaultKeyStyle.pressed.background);
    if (c.isValid()) {
        m_style.defaultKeyStyle.pressed.background = c;
        m_pressedColorEdit->setText(c.name());
    }
}

void KeyboardStyleDialog::onMouseInnerColorClicked() {
    QColor c = pickColor(m_style.defaultMouseIndicatorStyle.innerColor);
    if (c.isValid()) {
        m_style.defaultMouseIndicatorStyle.innerColor = c;
        m_mouseInnerColorEdit->setText(c.name());
    }
}

void KeyboardStyleDialog::onMouseOuterColorClicked() {
    QColor c = pickColor(m_style.defaultMouseIndicatorStyle.outerColor);
    if (c.isValid()) {
        m_style.defaultMouseIndicatorStyle.outerColor = c;
        m_mouseOuterColorEdit->setText(c.name());
    }
}

void KeyboardStyleDialog::onLooseOutlineColorClicked() {
    QColor c = pickColor(m_style.defaultKeyStyle.loose.outline);
    if (c.isValid()) {
        m_style.defaultKeyStyle.loose.outline = c;
        m_looseOutlineColorEdit->setText(c.name());
    }
}

void KeyboardStyleDialog::onPressedOutlineColorClicked() {
    QColor c = pickColor(m_style.defaultKeyStyle.pressed.outline);
    if (c.isValid()) {
        m_style.defaultKeyStyle.pressed.outline = c;
        m_pressedOutlineColorEdit->setText(c.name());
    }
}

void KeyboardStyleDialog::onLooseTextColorClicked() {
    QColor c = pickColor(m_style.defaultKeyStyle.loose.text);
    if (c.isValid()) {
        m_style.defaultKeyStyle.loose.text = c;
        m_looseTextColorEdit->setText(c.name());
    }
}

void KeyboardStyleDialog::onPressedTextColorClicked() {
    QColor c = pickColor(m_style.defaultKeyStyle.pressed.text);
    if (c.isValid()) {
        m_style.defaultKeyStyle.pressed.text = c;
        m_pressedTextColorEdit->setText(c.name());
    }
}