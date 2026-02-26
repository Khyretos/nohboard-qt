#pragma once
#include "models/keyboarddefinition.h"
#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <memory>

class ElementPropertiesDialog : public QDialog {
    Q_OBJECT
  public:
    explicit ElementPropertiesDialog(std::shared_ptr<ElementDefinition> element, int keyboardWidth, int keyboardHeight, QWidget *parent = nullptr);
    ~ElementPropertiesDialog() override;
    std::shared_ptr<ElementDefinition> getElement() const { return m_element; }

  public slots:
    void accept() override;

  private slots:
    void onAddBoundary();
    void onRemoveBoundary();
    void onMoveBoundaryUp();
    void onMoveBoundaryDown();
    void onAddKeyCode();
    void onRemoveKeyCode();
    void onMoveKeyCodeUp();
    void onMoveKeyCodeDown();
    void onCenterText();

  private:
    std::shared_ptr<ElementDefinition> m_element;
    int m_kbWidth;
    int m_kbHeight;

    QLineEdit *m_textEdit;
    QCheckBox *m_changeOnCapsCheck;
    QLineEdit *m_shiftTextEdit;
    QSpinBox *m_textXSpin;
    QSpinBox *m_textYSpin;
    QListWidget *m_boundariesList;
    QListWidget *m_keyCodesList;

    QSpinBox *m_newBoundaryX;
    QSpinBox *m_newBoundaryY;
    QSpinBox *m_newKeyCode;

    void setupUi();
    void updateBoundariesList();
    void updateKeyCodesList();
};