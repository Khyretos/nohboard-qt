#include "keyboardloader.h"
#include "../logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QProcessEnvironment>
#include <QStandardPaths>

static const QString COMP = "KeyboardLoader";

static bool copyRecursively(const QString &src, const QString &dst) {
    qDebug() << "copyRecursively from" << src << "to" << dst;
    QDir srcDir(src);
    if (!srcDir.exists()) {
        qWarning() << "Source directory does not exist:" << src;
        return false;
    }

    QDir dstDir(dst);
    if (!dstDir.exists()) {
        if (!dstDir.mkpath(dst)) {
            qWarning() << "Failed to create destination directory:" << dst;
            return false;
        }
    }

    for (const QString &entry : srcDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString srcPath = srcDir.absoluteFilePath(entry);
        QString dstPath = dstDir.absoluteFilePath(entry);

        if (QFileInfo(srcPath).isDir()) {
            qDebug() << "Copying subdirectory:" << srcPath;
            if (!copyRecursively(srcPath, dstPath)) {
                qWarning() << "Failed to copy subdirectory:" << srcPath;
                return false;
            }
        } else {
            qDebug() << "Copying file:" << srcPath << "->" << dstPath;
            if (QFile::exists(dstPath)) {
                qDebug() << "Destination file already exists, skipping:" << dstPath;
                continue;
            }
            if (!QFile::copy(srcPath, dstPath)) {
                qWarning() << "Failed to copy file:" << srcPath;
                // Continue with other files instead of failing completely
            }
        }
    }
    return true;
}

static QString findBundledKeyboardsPath() {
    QString exePath = QCoreApplication::applicationDirPath();
    qDebug() << "Executable path:" << exePath;

#ifdef Q_OS_MAC
    // macOS bundle detection: if we are inside a .app bundle, the path contains ".app/Contents/MacOS"
    if (exePath.contains(".app/Contents/MacOS")) {
        // Bundled keyboards should be in ../Resources/keyboards
        QString candidate = exePath + "/../Resources/keyboards";
        if (QDir(candidate).exists()) {
            return QDir::cleanPath(candidate);
        }
    }
#endif

    // First check resource system
    if (QDir(":/keyboards").exists()) {
        qDebug() << "Found bundled keyboards in resources: :/keyboards";
        return ":/keyboards";
    }

    // Then try next to executable (portable layout)
    QString candidate = exePath + "/keyboards";
    if (QDir(candidate).exists()) {
        qDebug() << "Found bundled keyboards next to executable:" << candidate;
        return QDir::cleanPath(candidate);
    }

    // Then try ../keyboards (AppImage layout)
    candidate = exePath + "/../keyboards";
    if (QDir(candidate).exists()) {
        qDebug() << "Found bundled keyboards in parent directory:" << QDir::cleanPath(candidate);
        return QDir::cleanPath(candidate);
    }

    // Fallback: current working directory (development)
    if (QDir("keyboards").exists()) {
        qDebug() << "Found bundled keyboards in current directory:" << QDir("keyboards").absolutePath();
        return QDir("keyboards").absolutePath();
    }

    qWarning() << "No bundled keyboards found!";
    return QString();
}

KeyboardLoader::KeyboardLoader(const QString &keyboardsRoot) : m_root(keyboardsRoot) {
    LOG_INFO(COMP, QString("📁 Keyboard root: %1").arg(m_root));
    scan();
}

void KeyboardLoader::scan() {
    m_entries.clear();
    QDir root(m_root);
    if (!root.exists()) {
        LOG_WARN(COMP, QString("⚠️  Keyboards root does not exist: %1").arg(m_root));
        return;
    }

    // Iterate category directories
    for (const QString &cat : root.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (cat == "global") continue; // styles only

        QDir catDir(root.filePath(cat));
        for (const QString &kbName : catDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QDir kbDir(catDir.filePath(kbName));
            QString defPath = kbDir.filePath("keyboard.json");

            if (!QFile::exists(defPath)) {
                LOG_DEBUG(COMP, QString("  📂 Skipping %1/%2 — no keyboard.json").arg(cat, kbName));
                continue;
            }

            KeyboardEntry entry;
            entry.category = cat;
            entry.keyboardName = kbName;
            entry.definitionPath = defPath;

            // Find .style files
            QStringList styles = kbDir.entryList({"*.style"}, QDir::Files);
            for (const QString &sf : styles) {
                entry.styleNames << sf.left(sf.length() - 6); // remove ".style"
            }

            m_entries.append(entry);
            LOG_DEBUG(COMP, QString("  ✅ Found: %1/%2 — %3 styles")
                                .arg(cat, kbName)
                                .arg(entry.styleNames.size()));
        }
    }

    LOG_INFO(COMP, QString("📋 Scan complete — %1 keyboards found").arg(m_entries.size()));
}

QStringList KeyboardLoader::categories() const {
    QSet<QString> cats;
    for (const auto &e : m_entries)
        cats.insert(e.category);
    QStringList list = cats.values();
    list.sort();
    return list;
}

QList<KeyboardEntry> KeyboardLoader::keyboardsInCategory(const QString &category) const {
    QList<KeyboardEntry> result;
    for (const auto &e : m_entries)
        if (e.category == category) result.append(e);
    return result;
}

QList<KeyboardEntry> KeyboardLoader::allKeyboards() const {
    return m_entries;
}

QString KeyboardLoader::getDefaultKeyboardsPath() {
    qDebug() << "===== KeyboardLoader::getDefaultKeyboardsPath() =====";

    // 1. Standard user data location (cross-platform)
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    qDebug() << "AppLocalDataLocation:" << dataPath;

    QString userKeyboardPath = dataPath + "/keyboards";
    qDebug() << "User keyboards path:" << userKeyboardPath;

    // Ensure the user directory exists
    QDir userDir(userKeyboardPath);
    if (!userDir.exists()) {
        qDebug() << "User keyboards directory does not exist, creating...";
        if (!userDir.mkpath(userKeyboardPath)) {
            qWarning() << "Failed to create user keyboards directory:" << userKeyboardPath;
            // Fallback: if we can't create, just return the bundled path directly
            QString fallbackBundled = findBundledKeyboardsPath();
            return fallbackBundled;
        } else {
            qDebug() << "Successfully created user keyboards directory.";
        }
    } else {
        qDebug() << "User keyboards directory already exists.";
    }

    // 2. Check if user directory is empty (first run)
    QStringList entries = userDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
    qDebug() << "User directory entries count:" << entries.size();

    if (entries.isEmpty()) {
        qDebug() << "User directory is empty, attempting to copy default keyboards.";

        // Find bundled keyboards path using platform-aware function
        QString bundledPath = findBundledKeyboardsPath();
        if (bundledPath.isEmpty()) {
            qWarning() << "Could not find bundled keyboards to copy.";
            return userKeyboardPath; // return user path (empty)
        }

        qDebug() << "Copying default keyboards from" << bundledPath << "to" << userKeyboardPath;
        // List contents of bundled path before copy
        QDir srcDir(bundledPath);
        QStringList srcEntries = srcDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
        qDebug() << "Bundled directory contains:" << srcEntries;

        if (copyRecursively(bundledPath, userKeyboardPath)) {
            qDebug() << "Default keyboards copied successfully.";
        } else {
            qWarning() << "Failed to copy default keyboards.";
        }
    } else {
        qDebug() << "User directory already has contents, skipping copy.";
    }

    qDebug() << "Returning user path:" << userKeyboardPath;
    return userKeyboardPath;
}