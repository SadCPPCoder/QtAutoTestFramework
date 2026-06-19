#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT

public:
    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    static Logger* instance();

    void setLogLevel(LogLevel level);
    void setLogFile(const QString &filePath);

    void debug(const QString &message);
    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    void log(LogLevel level, const QString &message);
    QString levelToString(LogLevel level) const;
    QString currentTimestamp() const;

    static Logger* s_instance;
    LogLevel m_logLevel = Info;
    QFile m_logFile;
    QTextStream m_fileStream;
    QMutex m_mutex;
};

#define LOG_DEBUG(msg) Logger::instance()->debug(msg)
#define LOG_INFO(msg) Logger::instance()->info(msg)
#define LOG_WARNING(msg) Logger::instance()->warning(msg)
#define LOG_ERROR(msg) Logger::instance()->error(msg)

#endif // LOGGER_H
