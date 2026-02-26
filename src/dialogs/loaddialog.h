#pragma once
#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QLabel>
#include "../loaders/keyboardloader.h"

class LoadDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoadDialog(KeyboardLoader* loader, QWidget* parent = nullptr);

    QString selectedDefinitionPath() const;
    QString selectedStylePath() const;

private slots:
    void onCategoryChanged(int index);
    void onKeyboardSelected(QListWidgetItem* item);

private:
    KeyboardLoader* m_loader;
    QComboBox* m_categoryCombo;
    QListWidget* m_keyboardList;
    QListWidget* m_styleList;
    QLabel* m_infoLabel;

    QList<KeyboardEntry> m_currentKeyboards;

    void setupUi();
    void populateCategories();
};
