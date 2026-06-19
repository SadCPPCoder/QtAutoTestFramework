#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <http.h>

class HttpServer : public QThread
{
    Q_OBJECT

public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer();

    bool startServer(quint16 port);
    void stop();

    void sendResponse(quint64 requestId, int statusCode, const QByteArray &body);
    void sendJsonResponse(quint64 requestId, int statusCode, const QByteArray &jsonBody);

signals:
    void requestReceived(quint64 requestId, const QString &method,
                        const QString &path, const QByteArray &body);

protected:
    void run() override;

private:
    QString parseMethod(HTTP_VERB verb);
    QString parseUrl(const HTTP_COOKED_URL &cookedUrl);
    QByteArray receiveBody(HANDLE requestQueue, HTTP_REQUEST_ID requestId);

    HANDLE m_requestQueue = INVALID_HANDLE_VALUE;
    std::atomic<bool> m_running{false};
    quint16 m_port = 0;
};

#endif // HTTPSERVER_H
