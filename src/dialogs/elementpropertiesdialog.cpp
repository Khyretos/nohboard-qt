#include "elementpropertiesdialog.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

ElementPropertiesDialog::ElementPropertiesDialog(std::shared_ptr<ElementDefinition> element, int keyboardWidth, int keyboardHeight, QWidget *parent)
    : QDialog(parent), m_element(element), m_kbWidth(keyboardWidth), m_kbHeight(keyboardHeight) {
    setWindowTitle("Element Properties");
    setupUi();
}

ElementPropertiesDialog::~ElementPropertiesDialog() {}

void ElementPropertiesDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- Text group ---
    QGroupBox *textGroup = new QGroupBox("Text");
    QFormLayout *textLayout = new QFormLayout(textGroup);

    m_textEdit = new QLineEdit(m_element->text);
    textLayout->addRow("Text:", m_textEdit);

    if (m_element->type == ElementType::KeyboardKey) {
        auto keyElem = std::dynamic_pointer_cast<KeyboardKeyDefinition>(m_element);
        if (keyElem) {
            m_changeOnCapsCheck = new QCheckBox();
            m_changeOnCapsCheck->setChecked(keyElem->changeOnCaps);
            textLayout->addRow("Change on Caps Lock:", m_changeOnCapsCheck);

            m_shiftTextEdit = new QLineEdit(keyElem->shiftText);
            textLayout->addRow("Shift Text:", m_shiftTextEdit);
        }
    }
    mainLayout->addWidget(textGroup);

    // --- Text Position group ---
    QGroupBox *posGroup = new QGroupBox("Text Position");
    QHBoxLayout *posLayout = new QHBoxLayout(posGroup);

    m_textXSpin = new QSpinBox();
    m_textXSpin->setRange(0, 5000);
    m_textXSpin->setValue(m_element->textPosition.x);
    m_textYSpin = new QSpinBox();
    m_textYSpin->setRange(0, 5000);
    m_textYSpin->setValue(m_element->textPosition.y);
    posLayout->addWidget(new QLabel("X:"));
    posLayout->addWidget(m_textXSpin);
    posLayout->addWidget(new QLabel("Y:"));
    posLayout->addWidget(m_textYSpin);

    QPushButton *centerBtn = new QPushButton("Center");
    connect(centerBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onCenterText);
    posLayout->addWidget(centerBtn);
    posLayout->addStretch();

    mainLayout->addWidget(posGroup);

    // --- Boundaries group (if applicable) ---
    if (m_element->type != ElementType::MouseSpeedIndicator) {
        QGroupBox *boundGroup = new QGroupBox("Boundaries");
        QVBoxLayout *boundLayout = new QVBoxLayout(boundGroup);

        // Input row for new boundary
        QHBoxLayout *inputLayout = new QHBoxLayout();
        inputLayout->addWidget(new QLabel("X:"));
        m_newBoundaryX = new QSpinBox();
        m_newBoundaryX->setRange(0, 5000);
        m_newBoundaryX->setValue(0);
        inputLayout->addWidget(m_newBoundaryX);
        inputLayout->addWidget(new QLabel("Y:"));
        m_newBoundaryY = new QSpinBox();
        m_newBoundaryY->setRange(0, 5000);
        m_newBoundaryY->setValue(0);
        inputLayout->addWidget(m_newBoundaryY);
        QPushButton *addBtn = new QPushButton("Add");
        connect(addBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onAddBoundary);
        inputLayout->addWidget(addBtn);
        inputLayout->addStretch();
        boundLayout->addLayout(inputLayout);

        // List of boundaries
        m_boundariesList = new QListWidget();
        updateBoundariesList();
        boundLayout->addWidget(m_boundariesList);

        // Buttons for manipulation
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *removeBtn = new QPushButton("Remove");
        QPushButton *upBtn = new QPushButton("Up");
        QPushButton *downBtn = new QPushButton("Down");

        connect(removeBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onRemoveBoundary);
        connect(upBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onMoveBoundaryUp);
        connect(downBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onMoveBoundaryDown);

        btnLayout->addWidget(removeBtn);
        btnLayout->addWidget(upBtn);
        btnLayout->addWidget(downBtn);
        btnLayout->addStretch();
        boundLayout->addLayout(btnLayout);

        mainLayout->addWidget(boundGroup);
    }

    // --- Key Codes group ---
    QGroupBox *codeGroup = new QGroupBox("Key Codes");
    QVBoxLayout *codeLayout = new QVBoxLayout(codeGroup);

    // Input row for new key code
    QHBoxLayout *codeInputLayout = new QHBoxLayout();
    codeInputLayout->addWidget(new QLabel("Code:"));
    m_newKeyCode = new QSpinBox();
    m_newKeyCode->setRange(0, 10000);
    m_newKeyCode->setValue(0);
    codeInputLayout->addWidget(m_newKeyCode);
    QPushButton *addCodeBtn = new QPushButton("Add");
    connect(addCodeBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onAddKeyCode);
    codeInputLayout->addWidget(addCodeBtn);
    codeInputLayout->addStretch();
    codeLayout->addLayout(codeInputLayout);

    // List of key codes
    m_keyCodesList = new QListWidget();
    updateKeyCodesList();
    codeLayout->addWidget(m_keyCodesList);

    // Buttons for manipulation
    QHBoxLayout *codeBtnLayout = new QHBoxLayout();
    QPushButton *removeCodeBtn = new QPushButton("Remove");
    QPushButton *upCodeBtn = new QPushButton("Up");
    QPushButton *downCodeBtn = new QPushButton("Down");

    connect(removeCodeBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onRemoveKeyCode);
    connect(upCodeBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onMoveKeyCodeUp);
    connect(downCodeBtn, &QPushButton::clicked, this, &ElementPropertiesDialog::onMoveKeyCodeDown);

    codeBtnLayout->addWidget(removeCodeBtn);
    codeBtnLayout->addWidget(upCodeBtn);
    codeBtnLayout->addWidget(downCodeBtn);
    codeBtnLayout->addStretch();
    codeLayout->addLayout(codeBtnLayout);

    mainLayout->addWidget(codeGroup);

    // --- Dialog buttons ---
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

// Helper to get boundaries vector (non-const) for modification
static QVector<TPoint> *getBoundaries(std::shared_ptr<ElementDefinition> elem) {
    if (elem->type == ElementType::KeyboardKey)
        return &(std::dynamic_pointer_cast<KeyboardKeyDefinition>(elem)->boundaries);
    else if (elem->type == ElementType::MouseKey)
        return &(std::dynamic_pointer_cast<MouseKeyDefinition>(elem)->boundaries);
    else if (elem->type == ElementType::MouseScroll)
        return &(std::dynamic_pointer_cast<MouseScrollDefinition>(elem)->boundaries);
    return nullptr;
}

void ElementPropertiesDialog::updateBoundariesList() {
    m_boundariesList->clear();
    auto *bounds = getBoundaries(m_element);
    if (!bounds) return;
    for (const auto &pt : *bounds) {
        m_boundariesList->addItem(QString("(%1, %2)").arg(pt.x).arg(pt.y));
    }
}

void ElementPropertiesDialog::updateKeyCodesList() {
    m_keyCodesList->clear();
    for (int code : m_element->keyCodes) {
        m_keyCodesList->addItem(QString::number(code));
    }
}

void ElementPropertiesDialog::onAddBoundary() {
    int x = m_newBoundaryX->value();
    int y = m_newBoundaryY->value();
    auto *bounds = getBoundaries(m_element);
    if (bounds) {
        bounds->append({x, y});
        updateBoundariesList();
        // Optionally reset input fields to default for convenience
        m_newBoundaryX->setValue(0);
        m_newBoundaryY->setValue(0);
    }
}

void ElementPropertiesDialog::onRemoveBoundary() {
    int row = m_boundariesList->currentRow();
    if (row < 0) return;
    auto *bounds = getBoundaries(m_element);
    if (bounds) {
        bounds->removeAt(row);
        updateBoundariesList();
    }
}

void ElementPropertiesDialog::onMoveBoundaryUp() {
    int row = m_boundariesList->currentRow();
    if (row <= 0) return;
    auto *bounds = getBoundaries(m_element);
    if (bounds) {
        bounds->swapItemsAt(row, row - 1);
        updateBoundariesList();
        m_boundariesList->setCurrentRow(row - 1);
    }
}

void ElementPropertiesDialog::onMoveBoundaryDown() {
    int row = m_boundariesList->currentRow();
    if (row < 0 || row >= m_boundariesList->count() - 1) return;
    auto *bounds = getBoundaries(m_element);
    if (bounds) {
        bounds->swapItemsAt(row, row + 1);
        updateBoundariesList();
        m_boundariesList->setCurrentRow(row + 1);
    }
}

void ElementPropertiesDialog::onAddKeyCode() {
    int code = m_newKeyCode->value();
    m_element->keyCodes.append(code);
    updateKeyCodesList();
    m_newKeyCode->setValue(0); // reset
}

void ElementPropertiesDialog::onRemoveKeyCode() {
    int row = m_keyCodesList->currentRow();
    if (row < 0) return;
    m_element->keyCodes.removeAt(row);
    updateKeyCodesList();
}

void ElementPropertiesDialog::onMoveKeyCodeUp() {
    int row = m_keyCodesList->currentRow();
    if (row <= 0) return;
    m_element->keyCodes.swapItemsAt(row, row - 1);
    updateKeyCodesList();
    m_keyCodesList->setCurrentRow(row - 1);
}

void ElementPropertiesDialog::onMoveKeyCodeDown() {
    int row = m_keyCodesList->currentRow();
    if (row < 0 || row >= m_keyCodesList->count() - 1) return;
    m_element->keyCodes.swapItemsAt(row, row + 1);
    updateKeyCodesList();
    m_keyCodesList->setCurrentRow(row + 1);
}

void ElementPropertiesDialog::onCenterText() {
    m_textXSpin->setValue(m_kbWidth / 2);
    m_textYSpin->setValue(m_kbHeight / 2);
}

void ElementPropertiesDialog::accept() {
    // Update text and text position
    m_element->text = m_textEdit->text();
    m_element->textPosition.x = m_textXSpin->value();
    m_element->textPosition.y = m_textYSpin->value();

    // Update keyboard-key-specific fields
    if (m_element->type == ElementType::KeyboardKey) {
        auto keyElem = std::dynamic_pointer_cast<KeyboardKeyDefinition>(m_element);
        if (keyElem) {
            keyElem->changeOnCaps = m_changeOnCapsCheck->isChecked();
            keyElem->shiftText = m_shiftTextEdit->text();
        }
    }

    QDialog::accept();
}