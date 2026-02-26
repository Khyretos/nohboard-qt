#pragma once
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

class Logger {
  public:
    static Logger &instance();

    void setLevel(LogLevel level);
    void setLogFile(const QString &path);
    void setConsoleOutput(bool enabled);

    void log(LogLevel level, const QString &component, const QString &message);

    void debug(const QString &component, const QString &msg) { log(LogLevel::DEBUG, component, msg); }
    void info(const QString &component, const QString &msg) { log(LogLevel::INFO, component, msg); }
    void warn(const QString &component, const QString &msg) { log(LogLevel::WARN, component, msg); }
    void error(const QString &component, const QString &msg) { log(LogLevel::ERROR, component, msg); }

  private:
    Logger() = default;
    ~Logger();

    LogLevel m_level = LogLevel::DEBUG;
    bool m_consoleOutput = true;
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;

    QString levelString(LogLevel level);
    QString levelEmoji(LogLevel level);
};

// Convenience macros
#define LOG_DEBUG(comp, msg) Logger::instance().debug(comp, msg)
#define LOG_INFO(comp, msg) Logger::instance().info(comp, msg)
#define LOG_WARN(comp, msg) Logger::instance().warn(comp, msg)
#define LOG_ERROR(comp, msg) Logger::instance().error(comp, msg)
