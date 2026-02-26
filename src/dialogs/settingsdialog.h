#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include "models/settings.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(const AppSettings &current, QWidget *parent = nullptr);
    AppSettings settings() const;

signals:
    void settingsApplied(const AppSettings &settings);

public slots:
    void accept() override;

private:
    // Input group
    QSpinBox *m_sensitivitySpin;
    QSpinBox *m_scrollHoldSpin;
    QCheckBox *m_mouseFromCenterCheck;
    QSpinBox *m_pressHoldSpin;

    // General group
    QLineEdit *m_windowTitleEdit;

    // Capitalization group
    QButtonGroup *m_capGroup;
    QRadioButton *m_capFollow;
    QRadioButton *m_capAllUpper;
    QRadioButton *m_capAllLower;
    QCheckBox *m_followInsensitive;
    QCheckBox *m_followSensitive;

    void setupUi(const AppSettings &s);
};