#include "Logger.h"
#include <QDebug>
#include <QDateTime>

Logger* Logger::s_instance = nullptr;

Logger* Logger::instance()
{
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
{
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_fileStream.flush();
        m_logFile.close();
    }
}

void Logger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

void Logger::setLogFile(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);

    if (m_logFile.isOpen()) {
        m_fileStream.flush();
        m_logFile.close();
    }

    m_logFile.setFileName(filePath);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_fileStream.setDevice(&m_logFile);
    } else {
        qWarning() << "Failed to open log file:" << filePath;
    }
}

void Logger::debug(const QString &message)
{
    log(Debug, message);
}

void Logger::info(const QString &message)
{
    log(Info, message);
}

void Logger::warning(const QString &message)
{
    log(Warning, message);
}

void Logger::error(const QString &message)
{
    log(Error, message);
}

void Logger::log(LogLevel level, const QString &message)
{
    if (level < m_logLevel) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    QString logMessage = QString("[%1] [%2] %3")
        .arg(currentTimestamp())
        .arg(levelToString(level))
        .arg(message);

    qDebug().noquote() << logMessage;

    if (m_logFile.isOpen()) {
        m_fileStream << logMessage << Qt::endl;
        m_fileStream.flush();
    }
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case Debug:   return "DEBUG";
        case Info:    return "INFO";
        case Warning: return "WARNING";
        case Error:   return "ERROR";
        default:      return "UNKNOWN";
    }
}

QString Logger::currentTimestamp() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}
