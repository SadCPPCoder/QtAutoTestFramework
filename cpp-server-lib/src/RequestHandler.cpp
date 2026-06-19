#include "RequestHandler.h"
#include "HttpServer.h"
#include "ObjectFinder.h"
#include "EventSender.h"
#include "ScreenCapture.h"
#include "JsonHelper.h"
#include "Logger.h"
#include <QJsonDocument>
#include <QUrl>
#include <QUrlQuery>
#include <QImage>
#include <QBuffer>
#include <QThread>
#include <QDateTime>

RequestHandler::RequestHandler(QQmlApplicationEngine *engine, QObject *parent)
    : QObject(parent)
{
    m_objectFinder = new ObjectFinder(engine, this);
    m_eventSender = new EventSender(this);
    m_screenCapture = new ScreenCapture(this);

    QList<QObject*> roots = engine->rootObjects();
    for (QObject *root : roots) {
        QQuickWindow *window = qobject_cast<QQuickWindow*>(root);
        if (window) {
            m_eventSender->setTargetWindow(window);
            m_screenCapture->setTargetWindow(window);
            break;
        }
    }
}

void RequestHandler::setHttpServer(HttpServer *server)
{
    m_httpServer = server;
}

void RequestHandler::updateWindow(QQuickWindow *window)
{
    if (window) {
        LOG_INFO("RequestHandler: Updating window reference");
        m_eventSender->setTargetWindow(window);
        m_screenCapture->setTargetWindow(window);
    }
}

void RequestHandler::handleRequest(quint64 requestId, const QString &method,
                                    const QString &path, const QByteArray &body)
{
    LOG_INFO(QString("%1 %2").arg(method).arg(path));

    if (path == "/api/v1/health" && method == "GET") {
        handleHealthCheck(requestId);
    }
    else if (path == "/api/v1/app/info" && method == "GET") {
        handleAppInfo(requestId);
    }
    else if (path == "/api/v1/objects/tree" && method == "GET") {
        handleObjectTree(requestId);
    }
    else if (path == "/api/v1/objects/query" && method == "POST") {
        handleObjectQuery(requestId, body);
    }
    else if (path.startsWith("/api/v1/objects/") && path.endsWith("/properties") && method == "GET") {
        QString objId = extractObjIdFromPath(path);
        handleGetProperties(requestId, objId);
    }
    else if (path.startsWith("/api/v1/objects/") && method == "GET") {
        QString objId = extractObjIdFromPath(path);
        handleGetObject(requestId, objId);
    }
    else if (path == "/api/v1/events" && method == "POST") {
        handleSendEvent(requestId, body);
    }
    else if (path == "/api/v1/screenshot" && method == "GET") {
        handleScreenshot(requestId, path);
    }
    else if (path == "/api/v1/wait" && method == "POST") {
        handleWait(requestId, body);
    }
    else {
        handleNotFound(requestId, path);
    }
}

void RequestHandler::handleHealthCheck(quint64 requestId)
{
    QJsonObject response;
    response["status"] = "ok";
    response["success"] = true;
    response["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    response["version"] = "1.0.0";

    sendSuccess(requestId, response);
}

void RequestHandler::handleAppInfo(quint64 requestId)
{
    QJsonObject info = m_objectFinder->getAppInfo();
    sendSuccess(requestId, info);
}

void RequestHandler::handleObjectTree(quint64 requestId)
{
    QJsonObject tree = m_objectFinder->getObjectTree();
    sendSuccess(requestId, tree);
}

void RequestHandler::handleObjectQuery(quint64 requestId, const QByteArray &body)
{
    QJsonObject query = JsonHelper::fromJsonBytes(body);

    if (query.isEmpty()) {
        sendError(requestId, 400, "INVALID_REQUEST", "Invalid JSON body");
        return;
    }

    QJsonArray objects = m_objectFinder->findObjects(query);

    QJsonObject response;
    response["objects"] = objects;
    response["count"] = objects.size();
    response["total"] = objects.size();

    sendSuccess(requestId, response);
}

void RequestHandler::handleGetObject(quint64 requestId, const QString &objId)
{
    if (objId.isEmpty()) {
        sendError(requestId, 400, "INVALID_REQUEST", "Missing object ID");
        return;
    }

    QJsonObject obj = m_objectFinder->findById(objId);

    if (obj.isEmpty()) {
        sendError(requestId, 404, "OBJECT_NOT_FOUND",
                  QString("Object not found: %1").arg(objId));
        return;
    }

    sendSuccess(requestId, obj);
}

void RequestHandler::handleGetProperties(quint64 requestId, const QString &objId)
{
    if (objId.isEmpty()) {
        sendError(requestId, 400, "INVALID_REQUEST", "Missing object ID");
        return;
    }

    QJsonObject obj = m_objectFinder->findById(objId);

    if (obj.isEmpty()) {
        sendError(requestId, 404, "OBJECT_NOT_FOUND",
                  QString("Object not found: %1").arg(objId));
        return;
    }

    QJsonObject response;
    response["objID"] = objId;
    response["properties"] = obj["properties"].toObject();

    sendSuccess(requestId, response);
}

void RequestHandler::handleSendEvent(quint64 requestId, const QByteArray &body)
{
    QJsonObject request = JsonHelper::fromJsonBytes(body);

    if (request.isEmpty() || !request.contains("event")) {
        sendError(requestId, 400, "INVALID_REQUEST", "Missing event");
        return;
    }

    QJsonObject event = request["event"].toObject();
    QString eventType = event["type"].toString();

    QJsonObject target = request["target"].toObject();
    QQuickItem *targetItem = nullptr;

    if (!target.isEmpty()) {
        targetItem = m_objectFinder->findQuickItem(target);
        if (targetItem) {
            LOG_INFO(QString("Found target item: %1").arg(targetItem->objectName()));
        } else {
            LOG_WARNING("Target item not found, sending to window");
        }
    }

    bool success = false;

    if (eventType == "mouse") {
        success = m_eventSender->sendMouseEvent(targetItem, event);
    } else if (eventType == "keyboard") {
        success = m_eventSender->sendKeyboardEvent(event);
    } else if (eventType == "touch") {
        success = m_eventSender->sendTouchEvent(event);
    } else if (eventType == "wheel") {
        int x = event["x"].toInt();
        int y = event["y"].toInt();
        int deltaX = event["deltaX"].toInt();
        int deltaY = event["deltaY"].toInt();
        success = m_eventSender->sendWheelEvent(x, y, deltaX, deltaY);
    } else {
        sendError(requestId, 400, "INVALID_EVENT",
                  QString("Unknown event type: %1").arg(eventType));
        return;
    }

    if (success) {
        QJsonObject response;
        response["success"] = true;
        response["eventId"] = QString("evt_%1").arg(QDateTime::currentMSecsSinceEpoch());
        response["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        sendSuccess(requestId, response);
    } else {
        sendError(requestId, 500, "EVENT_ERROR", "Failed to send event");
    }
}

void RequestHandler::handleScreenshot(quint64 requestId, const QString &queryString)
{
    QJsonObject result;

    QUrl url(queryString);
    QUrlQuery query(url);

    if (query.hasQueryItem("objID")) {
        QString objId = query.queryItemValue("objID");
        QJsonObject obj = m_objectFinder->findById(objId);

        if (obj.isEmpty()) {
            sendError(requestId, 404, "OBJECT_NOT_FOUND",
                      QString("Object not found: %1").arg(objId));
            return;
        }

        QQuickItem *item = m_objectFinder->findQuickItem(QJsonObject{{"objID", objId}});
        if (item) {
            result = m_screenCapture->captureObject(item);
        } else {
            result = m_screenCapture->captureWindow();
        }
    } else {
        result = m_screenCapture->captureWindow();
    }

    if (result.isEmpty()) {
        // Return a dummy image
        QImage dummyImage(1, 1, QImage::Format_ARGB32);
        dummyImage.fill(Qt::transparent);

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        dummyImage.save(&buffer, "PNG");

        result["image"] = QString::fromLatin1(ba.toBase64());
        result["width"] = 1;
        result["height"] = 1;
        result["format"] = "png";
        result["note"] = "Screenshot not available";
    }

    sendSuccess(requestId, result);
}

void RequestHandler::handleWait(quint64 requestId, const QByteArray &body)
{
    QJsonObject request = JsonHelper::fromJsonBytes(body);

    if (request.isEmpty() || !request.contains("condition")) {
        sendError(requestId, 400, "INVALID_REQUEST", "Missing condition");
        return;
    }

    QString condition = request["condition"].toString();
    int timeout = request["timeout"].toInt(5000);
    int interval = request["interval"].toInt(100);
    QJsonObject target = request["target"].toObject();

    QJsonObject response;

    if (condition == "object_appear") {
        qint64 startTime = QDateTime::currentMSecsSinceEpoch();
        while (QDateTime::currentMSecsSinceEpoch() - startTime < timeout) {
            QJsonObject obj = m_objectFinder->findObject(target);
            if (!obj.isEmpty()) {
                response["success"] = true;
                response["object"] = obj;
                response["elapsed"] = QDateTime::currentMSecsSinceEpoch() - startTime;
                sendSuccess(requestId, response);
                return;
            }
            QThread::msleep(interval);
        }
        sendError(requestId, 408, "TIMEOUT", "Timeout waiting for object");
    }
    else if (condition == "object_disappear") {
        qint64 startTime = QDateTime::currentMSecsSinceEpoch();
        while (QDateTime::currentMSecsSinceEpoch() - startTime < timeout) {
            QJsonObject obj = m_objectFinder->findObject(target);
            if (obj.isEmpty()) {
                response["success"] = true;
                response["elapsed"] = QDateTime::currentMSecsSinceEpoch() - startTime;
                sendSuccess(requestId, response);
                return;
            }
            QThread::msleep(interval);
        }
        sendError(requestId, 408, "TIMEOUT", "Timeout waiting for object to disappear");
    }
    else {
        QThread::msleep(qMin(timeout, 1000));
        response["success"] = true;
        response["elapsed"] = qMin(timeout, 1000);
        sendSuccess(requestId, response);
    }
}

void RequestHandler::handleNotFound(quint64 requestId, const QString &path)
{
    sendError(requestId, 404, "NOT_FOUND",
              QString("Endpoint not found: %1").arg(path));
}

void RequestHandler::sendJson(quint64 requestId, int statusCode, const QJsonObject &json)
{
    if (m_httpServer) {
        QByteArray body = JsonHelper::toJsonBytes(json);
        m_httpServer->sendJsonResponse(requestId, statusCode, body);
    }
}

void RequestHandler::sendSuccess(quint64 requestId, const QJsonObject &data)
{
    QJsonObject response = JsonHelper::successResponse(data);
    sendJson(requestId, 200, response);
}

void RequestHandler::sendError(quint64 requestId, int statusCode,
                                const QString &code, const QString &message)
{
    QJsonObject response = JsonHelper::errorResponse(code, message);
    sendJson(requestId, statusCode, response);
}

QString RequestHandler::extractObjIdFromPath(const QString &path)
{
    QStringList parts = path.split('/');

    int objectsIndex = parts.indexOf("objects");
    if (objectsIndex >= 0 && objectsIndex + 1 < parts.size()) {
        return parts[objectsIndex + 1];
    }

    return QString();
}
