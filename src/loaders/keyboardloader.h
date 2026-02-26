#pragma once
#include <QDir>
#include <QString>
#include <QStringList>

struct KeyboardEntry {
    QString category;
    QString keyboardName;
    QString definitionPath;
    QStringList styleNames;
    QString stylePath(const QString &styleName) const {
        QDir dir = QFileInfo(definitionPath).dir();
        return dir.filePath(styleName + ".style");
    }
};

class KeyboardLoader {
  public:
    explicit KeyboardLoader(const QString &keyboardsRoot);

    QStringList categories() const;
    QList<KeyboardEntry> keyboardsInCategory(const QString &category) const;
    QList<KeyboardEntry> allKeyboards() const;

    QString rootPath() const { return m_root; };
    static QString getDefaultKeyboardsPath();

  private:
    QString m_root;
    QList<KeyboardEntry> m_entries;

    void scan();
};
