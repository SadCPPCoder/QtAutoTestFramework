#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <QObject>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

class HttpServer;
class ObjectFinder;
class EventSender;
class ScreenCapture;

class RequestHandler : public QObject
{
    Q_OBJECT

public:
    explicit RequestHandler(QQmlApplicationEngine *engine, QObject *parent = nullptr);

    void setHttpServer(HttpServer *server);
    void updateWindow(QQuickWindow *window);

public slots:
    void handleRequest(quint64 requestId, const QString &method,
                      const QString &path, const QByteArray &body);

private:
    void handleHealthCheck(quint64 requestId);
    void handleAppInfo(quint64 requestId);
    void handleObjectTree(quint64 requestId);
    void handleObjectQuery(quint64 requestId, const QByteArray &body);
    void handleGetObject(quint64 requestId, const QString &objId);
    void handleGetProperties(quint64 requestId, const QString &objId);
    void handleSendEvent(quint64 requestId, const QByteArray &body);
    void handleScreenshot(quint64 requestId, const QString &queryString);
    void handleWait(quint64 requestId, const QByteArray &body);
    void handleNotFound(quint64 requestId, const QString &path);

    void sendJson(quint64 requestId, int statusCode, const QJsonObject &json);
    void sendSuccess(quint64 requestId, const QJsonObject &data = QJsonObject());
    void sendError(quint64 requestId, int statusCode, const QString &code, const QString &message);
    QString extractObjIdFromPath(const QString &path);

    HttpServer *m_httpServer = nullptr;
    ObjectFinder *m_objectFinder;
    EventSender *m_eventSender;
    ScreenCapture *m_screenCapture;
};

#endif // REQUESTHANDLER_H
