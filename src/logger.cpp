#include "logger.h"
#include <QTextStream>
#include <iostream>

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::~Logger() {
    if (m_logFile.isOpen()) {
        m_logStream.flush();
        m_logFile.close();
    }
}

void Logger::setLevel(LogLevel level) {
    QMutexLocker lock(&m_mutex);
    m_level = level;
}

void Logger::setLogFile(const QString& path) {
    QMutexLocker lock(&m_mutex);
    if (m_logFile.isOpen()) m_logFile.close();
    m_logFile.setFileName(path);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
        std::cout << "[Logger] 📄 Log file opened: " << path.toStdString() << "\n";
    } else {
        std::cerr << "[Logger] ❌ Failed to open log file: " << path.toStdString() << "\n";
    }
}

void Logger::setConsoleOutput(bool enabled) {
    QMutexLocker lock(&m_mutex);
    m_consoleOutput = enabled;
}

QString Logger::levelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default:              return "?????";
    }
}

QString Logger::levelEmoji(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "🔍";
        case LogLevel::INFO:  return "ℹ️ ";
        case LogLevel::WARN:  return "⚠️ ";
        case LogLevel::ERROR: return "❌";
        default:              return "❓";
    }
}

void Logger::log(LogLevel level, const QString& component, const QString& message) {
    if (level < m_level) return;

    QMutexLocker lock(&m_mutex);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString line = QString("[%1] %2 %3 [%4] %5")
        .arg(timestamp)
        .arg(levelEmoji(level))
        .arg(levelString(level))
        .arg(component.leftJustified(20))
        .arg(message);

    if (m_consoleOutput) {
        if (level >= LogLevel::ERROR)
            std::cerr << line.toStdString() << "\n";
        else
            std::cout << line.toStdString() << "\n";
    }

    if (m_logFile.isOpen()) {
        m_logStream << line << "\n";
        m_logStream.flush();
    }
}
