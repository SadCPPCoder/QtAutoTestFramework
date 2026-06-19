#include "JsonHelper.h"

QJsonObject JsonHelper::successResponse(const QJsonObject &data)
{
    QJsonObject response;
    response["success"] = true;

    if (!data.isEmpty()) {
        for (auto it = data.begin(); it != data.end(); ++it) {
            response[it.key()] = it.value();
        }
    }

    return response;
}

QJsonObject JsonHelper::errorResponse(const QString &code, const QString &message,
                                       const QJsonObject &details)
{
    QJsonObject response;
    response["success"] = false;

    QJsonObject error;
    error["code"] = code;
    error["message"] = message;

    if (!details.isEmpty()) {
        error["details"] = details;
    }

    response["error"] = error;
    return response;
}

QJsonObject JsonHelper::paginatedResponse(const QJsonArray &items, int total,
                                            int offset, int limit)
{
    QJsonObject response;
    response["objects"] = items;
    response["count"] = items.size();
    response["total"] = total;
    response["offset"] = offset;
    response["limit"] = limit;
    return response;
}

QJsonValue JsonHelper::variantToJson(const QVariant &value)
{
    if (value.isNull() || !value.isValid()) {
        return QJsonValue::Null;
    }

    switch (value.typeId()) {
        case QMetaType::Bool:
            return value.toBool();
        case QMetaType::Int:
        case QMetaType::LongLong:
            return static_cast<qint64>(value.toLongLong());
        case QMetaType::UInt:
        case QMetaType::ULongLong:
            return static_cast<qint64>(value.toULongLong());
        case QMetaType::Double:
        case QMetaType::Float:
            return value.toDouble();
        case QMetaType::QString:
            return value.toString();
        case QMetaType::QStringList: {
            QJsonArray array;
            for (const QString &s : value.toStringList()) {
                array.append(s);
            }
            return array;
        }
        default:
            return value.toString();
    }
}

QByteArray JsonHelper::toJsonBytes(const QJsonObject &json)
{
    return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

QByteArray JsonHelper::toJsonBytes(const QJsonArray &json)
{
    return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

QJsonObject JsonHelper::fromJsonBytes(const QByteArray &bytes)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &error);

    if (error.error != QJsonParseError::NoError) {
        return QJsonObject();
    }

    return doc.object();
}

QJsonObject JsonHelper::objectInfoJson(const QString &objId,
                                         const QString &objectName,
                                         const QString &className,
                                         const QJsonObject &properties,
                                         const QJsonObject &geometry,
                                         bool visible,
                                         bool enabled,
                                         const QString &path)
{
    QJsonObject info;

    if (!objId.isEmpty()) {
        info["objID"] = objId;
    }
    info["objectName"] = objectName;
    info["className"] = className;
    info["properties"] = properties;
    info["geometry"] = geometry;
    info["visible"] = visible;
    info["enabled"] = enabled;
    info["path"] = path;

    return info;
}

QJsonObject JsonHelper::geometryJson(double x, double y, double width, double height)
{
    QJsonObject geometry;
    geometry["x"] = x;
    geometry["y"] = y;
    geometry["width"] = width;
    geometry["height"] = height;
    return geometry;
}
