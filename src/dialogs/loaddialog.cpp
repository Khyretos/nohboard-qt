#include "loaddialog.h"
#include "../logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSplitter>

static const QString COMP = "LoadDialog";

LoadDialog::LoadDialog(KeyboardLoader* loader, QWidget* parent)
    : QDialog(parent), m_loader(loader)
{
    setWindowTitle("🎹  Load Keyboard");
    setMinimumSize(600, 400);
    setupUi();
    populateCategories();
}

void LoadDialog::setupUi() {
    setStyleSheet(R"(
        QDialog { background: #1e1e1e; color: #ddd; }
        QLabel  { color: #ddd; }
        QComboBox, QListWidget {
            background: #2a2a2a; color: #eee;
            border: 1px solid #444; border-radius: 4px;
        }
        QComboBox::drop-down { border: none; }
        QListWidget::item:selected { background: #3a6fd8; }
        QPushButton {
            background: #3a6fd8; color: #fff;
            border: none; border-radius: 4px;
            padding: 6px 16px;
        }
        QPushButton:hover { background: #5080e0; }
        QPushButton:disabled { background: #555; color: #888; }
        QDialogButtonBox QPushButton { min-width: 80px; }
    )");

    auto* mainLayout = new QVBoxLayout(this);

    // Category
    auto* catLayout = new QHBoxLayout();
    catLayout->addWidget(new QLabel("📁  Category:"));
    m_categoryCombo = new QComboBox();
    catLayout->addWidget(m_categoryCombo, 1);
    mainLayout->addLayout(catLayout);

    // Keyboard + Style lists
    auto* splitter = new QSplitter(Qt::Horizontal);

    auto* kbGroup = new QWidget();
    auto* kbLayout = new QVBoxLayout(kbGroup);
    kbLayout->setContentsMargins(0,0,0,0);
    kbLayout->addWidget(new QLabel("⌨️  Keyboard:"));
    m_keyboardList = new QListWidget();
    kbLayout->addWidget(m_keyboardList);
    splitter->addWidget(kbGroup);

    auto* styleGroup = new QWidget();
    auto* styleLayout = new QVBoxLayout(styleGroup);
    styleLayout->setContentsMargins(0,0,0,0);
    styleLayout->addWidget(new QLabel("🎨  Style:"));
    m_styleList = new QListWidget();
    styleLayout->addWidget(m_styleList);
    splitter->addWidget(styleGroup);

    mainLayout->addWidget(splitter, 1);

    // Info label
    m_infoLabel = new QLabel("Select a keyboard to load");
    m_infoLabel->setStyleSheet("color: #888; font-size: 10px;");
    mainLayout->addWidget(m_infoLabel);

    // Buttons
    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox);

    // Connections
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoadDialog::onCategoryChanged);
    connect(m_keyboardList, &QListWidget::itemClicked,
            this, &LoadDialog::onKeyboardSelected);
    connect(m_keyboardList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem*) { accept(); });
}

void LoadDialog::populateCategories() {
    m_categoryCombo->clear();
    for (const QString& cat : m_loader->categories()) {
        m_categoryCombo->addItem(cat);
    }
    if (m_categoryCombo->count() > 0)
        onCategoryChanged(0);
}

void LoadDialog::onCategoryChanged(int index) {
    QString cat = m_categoryCombo->itemText(index);
    LOG_DEBUG(COMP, QString("Category selected: %1").arg(cat));

    m_keyboardList->clear();
    m_styleList->clear();
    m_currentKeyboards = m_loader->keyboardsInCategory(cat);

    for (const auto& kb : m_currentKeyboards) {
        m_keyboardList->addItem(kb.keyboardName);
    }
}

void LoadDialog::onKeyboardSelected(QListWidgetItem* item) {
    int row = m_keyboardList->row(item);
    if (row < 0 || row >= m_currentKeyboards.size()) return;

    const KeyboardEntry& kb = m_currentKeyboards[row];
    m_styleList->clear();
    m_styleList->addItem("(default)");
    for (const QString& s : kb.styleNames) {
        m_styleList->addItem(s);
    }
    m_styleList->setCurrentRow(0);

    m_infoLabel->setText(QString("📂 %1  →  %2").arg(kb.category, kb.keyboardName));
}

QString LoadDialog::selectedDefinitionPath() const {
    int row = m_keyboardList->currentRow();
    if (row < 0 || row >= m_currentKeyboards.size()) return {};
    return m_currentKeyboards[row].definitionPath;
}

QString LoadDialog::selectedStylePath() const {
    int kbRow = m_keyboardList->currentRow();
    if (kbRow < 0 || kbRow >= m_currentKeyboards.size()) return {};

    int styleRow = m_styleList->currentRow();
    if (styleRow <= 0) return {}; // "(default)"

    QString styleName = m_styleList->currentItem()->text();
    return m_currentKeyboards[kbRow].stylePath(styleName);
}
