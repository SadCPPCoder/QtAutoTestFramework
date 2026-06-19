#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariant>

class JsonHelper
{
public:
    static QJsonObject successResponse(const QJsonObject &data = QJsonObject());
    static QJsonObject errorResponse(const QString &code, const QString &message,
                                     const QJsonObject &details = QJsonObject());
    static QJsonObject paginatedResponse(const QJsonArray &items, int total,
                                          int offset = 0, int limit = 10);
    static QJsonValue variantToJson(const QVariant &value);
    static QByteArray toJsonBytes(const QJsonObject &json);
    static QByteArray toJsonBytes(const QJsonArray &json);
    static QJsonObject fromJsonBytes(const QByteArray &bytes);
    static QJsonObject objectInfoJson(const QString &objId,
                                       const QString &objectName,
                                       const QString &className,
                                       const QJsonObject &properties,
                                       const QJsonObject &geometry,
                                       bool visible,
                                       bool enabled,
                                       const QString &path);
    static QJsonObject geometryJson(double x, double y, double width, double height);
};

#endif // JSONHELPER_H
