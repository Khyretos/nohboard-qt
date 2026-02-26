#pragma once
#include <QDialog>
#include <QSpinBox>

class KeyboardPropertiesDialog : public QDialog {
    Q_OBJECT
  public:
    KeyboardPropertiesDialog(int width, int height, QWidget *parent = nullptr);
    int width() const;
    int height() const;

  private:
    QSpinBox *m_widthSpin;
    QSpinBox *m_heightSpin;
};