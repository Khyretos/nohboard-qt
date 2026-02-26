#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(const AppSettings &current, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("⚙️ Settings");
    setMinimumSize(400, 500);
    setupUi(current);
}

void SettingsDialog::setupUi(const AppSettings &s)
{
    setStyleSheet(R"(
        QDialog { background: #1e1e1e; color: #ddd; }
        QGroupBox { color: #ddd; border: 1px solid #444; border-radius: 4px; margin-top: 8px; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
        QLabel, QCheckBox, QRadioButton { color: #ddd; }
        QSpinBox, QLineEdit {
            background: #2a2a2a; color: #eee;
            border: 1px solid #444; border-radius: 4px; padding: 2px 4px;
        }
        QPushButton {
            background: #3a6fd8; color: #fff;
            border: none; border-radius: 4px; padding: 6px 16px;
        }
        QPushButton:hover { background: #5080e0; }
        QCheckBox::indicator { width: 16px; height: 16px; }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Input group
    QGroupBox *inputGroup = new QGroupBox("Input");
    QFormLayout *inputLayout = new QFormLayout(inputGroup);

    m_sensitivitySpin = new QSpinBox();
    m_sensitivitySpin->setRange(1, 500);
    m_sensitivitySpin->setValue(s.mouseSensitivity);
    m_sensitivitySpin->setSuffix("  (lower = more sensitive)");
    inputLayout->addRow("Mouse sensitivity:", m_sensitivitySpin);

    m_scrollHoldSpin = new QSpinBox();
    m_scrollHoldSpin->setRange(0, 500);
    m_scrollHoldSpin->setValue(s.scrollHold);
    m_scrollHoldSpin->setSuffix(" ms");
    inputLayout->addRow("Scroll hold time:", m_scrollHoldSpin);

    m_mouseFromCenterCheck = new QCheckBox();
    m_mouseFromCenterCheck->setChecked(s.mouseFromCenter);
    inputLayout->addRow("Calculate mouse speed from center of screen:", m_mouseFromCenterCheck);

    m_pressHoldSpin = new QSpinBox();
    m_pressHoldSpin->setRange(0, 1000);
    m_pressHoldSpin->setValue(s.pressHold);
    m_pressHoldSpin->setSuffix(" ms");
    inputLayout->addRow("Show keypress for at least:", m_pressHoldSpin);

    mainLayout->addWidget(inputGroup);

    // General group
    QGroupBox *generalGroup = new QGroupBox("General");
    QFormLayout *generalLayout = new QFormLayout(generalGroup);

    m_windowTitleEdit = new QLineEdit();
    m_windowTitleEdit->setText(s.windowTitle);
    generalLayout->addRow("Window Title:", m_windowTitleEdit);

    mainLayout->addWidget(generalGroup);

    // Capitalization group
    QGroupBox *capGroup = new QGroupBox("Capitalization of Keys");
    QVBoxLayout *capLayout = new QVBoxLayout(capGroup);

    m_capFollow = new QRadioButton("Follow Caps-Lock and Shift");
    m_capAllUpper = new QRadioButton("Show all buttons capitalized");
    m_capAllLower = new QRadioButton("Show all buttons lower-case");

    m_capGroup = new QButtonGroup(this);
    m_capGroup->addButton(m_capFollow, 0);
    m_capGroup->addButton(m_capAllUpper, 1);
    m_capGroup->addButton(m_capAllLower, 2);

    if (s.capitalization == 0)
        m_capFollow->setChecked(true);
    else if (s.capitalization == 1)
        m_capAllUpper->setChecked(true);
    else
        m_capAllLower->setChecked(true);

    capLayout->addWidget(m_capFollow);
    capLayout->addWidget(m_capAllUpper);
    capLayout->addWidget(m_capAllLower);

    QHBoxLayout *followLayout = new QHBoxLayout();
    followLayout->addWidget(new QLabel("Still follow shift for:"));
    m_followInsensitive = new QCheckBox("Caps Lock insensitive keys");
    m_followInsensitive->setChecked(s.followShiftForCapsInsensitive);
    m_followSensitive = new QCheckBox("Caps Lock sensitive keys");
    m_followSensitive->setChecked(s.followShiftForCapsSensitive);
    followLayout->addWidget(m_followInsensitive);
    followLayout->addWidget(m_followSensitive);
    followLayout->addStretch();
    capLayout->addLayout(followLayout);

    mainLayout->addWidget(capGroup);

    mainLayout->addStretch();

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox);
}

AppSettings SettingsDialog::settings() const
{
    AppSettings s;
    s.mouseSensitivity = m_sensitivitySpin->value();
    s.scrollHold = m_scrollHoldSpin->value();
    s.mouseFromCenter = m_mouseFromCenterCheck->isChecked();
    s.pressHold = m_pressHoldSpin->value();
    s.windowTitle = m_windowTitleEdit->text();
    s.capitalization = m_capGroup->checkedId();
    s.followShiftForCapsInsensitive = m_followInsensitive->isChecked();
    s.followShiftForCapsSensitive = m_followSensitive->isChecked();
    // Other settings (updateInterval, trap, etc.) remain unchanged
    return s;
}

void SettingsDialog::accept()
{
    emit settingsApplied(settings());
    QDialog::accept();
}