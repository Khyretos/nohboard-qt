#include "styledialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

StyleDialog::StyleDialog(const KeyboardStyle& style, QWidget* parent)
    : QDialog(parent), m_style(style)
{
    setWindowTitle("🎨  Style Editor");
    setMinimumSize(400, 300);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("🎨  Style editor — coming soon!\n\nEdit .style JSON files directly for now."));

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(btnBox);
}
