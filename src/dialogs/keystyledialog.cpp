#include "keystyledialog.h"
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

KeyStyleDialog::KeyStyleDialog(const KeyStyle &style, bool isOverride, QWidget *parent)
    : QDialog(parent), m_style(style), m_isOverride(isOverride) {
    setWindowTitle("Key Style");
    setupUi();
    updateFromStyle();
    m_overwriteCheck->setChecked(isOverride);
}

KeyStyleDialog::~KeyStyleDialog() {}

void KeyStyleDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Overwrite checkbox
    m_overwriteCheck = new QCheckBox("Overwrite default style");
    connect(m_overwriteCheck, &QCheckBox::toggled, this, &KeyStyleDialog::onOverwriteToggled);
    onOverwriteToggled(m_overwriteCheck->isChecked());

    mainLayout->addWidget(m_overwriteCheck);

    // --- Loose style group ---
    QGroupBox *looseGroup = new QGroupBox("Loose");
    QFormLayout *looseLayout = new QFormLayout(looseGroup);

    m_looseColorEdit = new QLineEdit();
    m_looseColorEdit->setReadOnly(true);
    QPushButton *looseColorBtn = new QPushButton("Choose...");
    connect(looseColorBtn, &QPushButton::clicked, this, &KeyStyleDialog::onLooseColorClicked);
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
    connect(looseOutlineBtn, &QPushButton::clicked, this, &KeyStyleDialog::onLooseOutlineColorClicked);
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
    connect(looseTextColorBtn, &QPushButton::clicked, this, &KeyStyleDialog::onLooseTextColorClicked);
    QHBoxLayout *looseTextLayout = new QHBoxLayout();
    looseTextLayout->addWidget(m_looseTextColorEdit);
    looseTextLayout->addWidget(looseTextColorBtn);
    looseLayout->addRow("Text Color:", looseTextLayout);

    mainLayout->addWidget(looseGroup);

    // --- Pressed style group (similar) ---
    QGroupBox *pressedGroup = new QGroupBox("Pressed");
    QFormLayout *pressedLayout = new QFormLayout(pressedGroup);

    m_pressedColorEdit = new QLineEdit();
    m_pressedColorEdit->setReadOnly(true);
    QPushButton *pressedColorBtn = new QPushButton("Choose...");
    connect(pressedColorBtn, &QPushButton::clicked, this, &KeyStyleDialog::onPressedColorClicked);
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
    connect(pressedOutlineBtn, &QPushButton::clicked, this, &KeyStyleDialog::onPressedOutlineColorClicked);
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
    connect(pressedTextColorBtn, &QPushButton::clicked, this, &KeyStyleDialog::onPressedTextColorClicked);
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

void KeyStyleDialog::updateFromStyle() {
    const auto &loose = m_style.loose;
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

    const auto &pressed = m_style.pressed;
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

QColor KeyStyleDialog::pickColor(const QColor &initial) {
    return QColorDialog::getColor(initial, this, "Choose Color");
}

void KeyStyleDialog::accept() {
    auto &loose = m_style.loose;
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

    auto &pressed = m_style.pressed;
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

void KeyStyleDialog::onLooseColorClicked() {
    QColor c = pickColor(m_style.loose.background);
    if (c.isValid()) {
        m_style.loose.background = c;
        m_looseColorEdit->setText(c.name());
    }
}

void KeyStyleDialog::onPressedColorClicked() {
    QColor c = pickColor(m_style.pressed.background);
    if (c.isValid()) {
        m_style.pressed.background = c;
        m_pressedColorEdit->setText(c.name());
    }
}

void KeyStyleDialog::onLooseOutlineColorClicked() {
    QColor c = pickColor(m_style.loose.outline);
    if (c.isValid()) {
        m_style.loose.outline = c;
        m_looseOutlineColorEdit->setText(c.name());
    }
}

void KeyStyleDialog::onPressedOutlineColorClicked() {
    QColor c = pickColor(m_style.pressed.outline);
    if (c.isValid()) {
        m_style.pressed.outline = c;
        m_pressedOutlineColorEdit->setText(c.name());
    }
}

void KeyStyleDialog::onLooseTextColorClicked() {
    QColor c = pickColor(m_style.loose.text);
    if (c.isValid()) {
        m_style.loose.text = c;
        m_looseTextColorEdit->setText(c.name());
    }
}

void KeyStyleDialog::onPressedTextColorClicked() {
    QColor c = pickColor(m_style.pressed.text);
    if (c.isValid()) {
        m_style.pressed.text = c;
        m_pressedTextColorEdit->setText(c.name());
    }
}

void KeyStyleDialog::onOverwriteToggled(bool checked) {
    // Enable/disable all editing widgets
    m_looseColorEdit->setEnabled(checked);
    m_looseImageEdit->setEnabled(checked);
    m_looseFontCombo->setEnabled(checked);
    m_looseFontSizeSpin->setEnabled(checked);
    m_looseBoldCheck->setEnabled(checked);
    m_looseItalicCheck->setEnabled(checked);
    m_looseOutlineColorEdit->setEnabled(checked);
    m_looseShowOutlineCheck->setEnabled(checked);
    m_looseOutlineWidthSpin->setEnabled(checked);
    m_looseTextColorEdit->setEnabled(checked);

    m_pressedColorEdit->setEnabled(checked);
    m_pressedImageEdit->setEnabled(checked);
    m_pressedFontCombo->setEnabled(checked);
    m_pressedFontSizeSpin->setEnabled(checked);
    m_pressedBoldCheck->setEnabled(checked);
    m_pressedItalicCheck->setEnabled(checked);
    m_pressedOutlineColorEdit->setEnabled(checked);
    m_pressedShowOutlineCheck->setEnabled(checked);
    m_pressedOutlineWidthSpin->setEnabled(checked);
    m_pressedTextColorEdit->setEnabled(checked);
}