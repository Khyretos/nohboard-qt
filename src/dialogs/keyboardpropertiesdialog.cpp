#include "keyboardpropertiesdialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

KeyboardPropertiesDialog::KeyboardPropertiesDialog(int w, int h, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Keyboard Properties");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();
    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(1, 5000);
    m_widthSpin->setValue(w);
    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(1, 5000);
    m_heightSpin->setValue(h);
    form->addRow("Width:", m_widthSpin);
    form->addRow("Height:", m_heightSpin);
    mainLayout->addLayout(form);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

int KeyboardPropertiesDialog::width() const { return m_widthSpin->value(); }
int KeyboardPropertiesDialog::height() const { return m_heightSpin->value(); }