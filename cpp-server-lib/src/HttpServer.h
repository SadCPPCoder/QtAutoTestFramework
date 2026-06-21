#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QHash>
#include <atomic>

/**
 * @brief Platform-agnostic HTTP Server
 *
 * This class provides an HTTP server that works on both Windows and Linux.
 * - Windows: Uses Windows HTTP Server API (http.h)
 * - Linux: Uses libmicrohttpd
 *
 * The public API is the same on both platforms.
 */
class HttpServer : public QThread
{
    Q_OBJECT

public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer();

    /**
     * @brief Start the HTTP server on the specified port
     * @param port The port to listen on
     * @return true if successful, false otherwise
     */
    bool startServer(quint16 port);

    /**
     * @brief Stop the HTTP server
     */
    void stop();

    /**
     * @brief Send an HTTP response
     * @param requestId The request ID to respond to
     * @param statusCode The HTTP status code
     * @param body The response body
     */
    void sendResponse(quint64 requestId, int statusCode, const QByteArray &body);

    /**
     * @brief Send a JSON HTTP response
     * @param requestId The request ID to respond to
     * @param statusCode The HTTP status code
     * @param jsonBody The JSON response body
     */
    void sendJsonResponse(quint64 requestId, int statusCode, const QByteArray &jsonBody);

signals:
    /**
     * @brief Emitted when an HTTP request is received
     * @param requestId Unique ID for this request
     * @param method HTTP method (GET, POST, etc.)
     * @param path Request path
     * @param body Request body (for POST/PUT)
     */
    void requestReceived(quint64 requestId, const QString &method,
                        const QString &path, const QByteArray &body);

protected:
    void run() override;

private:
    // Platform-specific implementation
    class Impl;
    Impl *m_impl;

    std::atomic<bool> m_running{false};
    quint16 m_port = 0;
};

#endif // HTTPSERVER_H
