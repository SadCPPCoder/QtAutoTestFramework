#include "ObjectFinder.h"
#include "JsonHelper.h"
#include "Logger.h"
#include <QQuickWindow>
#include <QMetaProperty>
#include <QSet>

ObjectFinder::ObjectFinder(QQmlApplicationEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
}

QJsonArray ObjectFinder::findObjects(const QJsonObject &query)
{
    QJsonArray results;
    QObjectList matches;
    QSet<QObject*> visited;

    QList<QObject*> roots = m_engine->rootObjects();
    for (QObject *root : roots) {
        findAllItems(root, query, matches, visited);
    }

    int offset = query.value("offset").toInt(0);
    int limit = query.value("limit").toInt(matches.size());

    int start = qMin(offset, matches.size());
    int end = qMin(start + limit, matches.size());

    for (int i = start; i < end; i++) {
        results.append(getObjectInfo(matches[i]));
    }

    return results;
}

QJsonObject ObjectFinder::findObject(const QJsonObject &query)
{
    QObjectList matches;
    QSet<QObject*> visited;

    QList<QObject*> roots = m_engine->rootObjects();
    for (QObject *root : roots) {
        findAllItems(root, query, matches, visited);
    }

    if (matches.isEmpty()) {
        return QJsonObject();
    }

    if (matches.size() > 1) {
        LOG_WARNING(QString("Found %1 objects, returning first").arg(matches.size()));
    }

    return getObjectInfo(matches.first());
}

QJsonObject ObjectFinder::findById(const QString &objID)
{
    QJsonObject query;
    query["objID"] = objID;
    return findObject(query);
}

QQuickItem *ObjectFinder::findQuickItem(const QJsonObject &query)
{
    QList<QObject*> roots = m_engine->rootObjects();
    for (QObject *root : roots) {
        QQuickItem *item = findFirstItem(root, query);
        if (item) {
            return item;
        }
    }
    return nullptr;
}

QQuickItem *ObjectFinder::findFirstItem(QObject *root, const QJsonObject &query)
{
    if (!root) return nullptr;

    if (matchObject(root, query)) {
        return qobject_cast<QQuickItem*>(root);
    }

    QList<QObject*> children = root->children();
    for (QObject *child : children) {
        QQuickItem *item = findFirstItem(child, query);
        if (item) return item;
    }

    if (isQuickItem(root)) {
        QQuickItem *quickItem = toQuickItem(root);
        QList<QQuickItem*> childItems = quickItem->childItems();
        for (QQuickItem *child : childItems) {
            if (!children.contains(child)) {
                QQuickItem *item = findFirstItem(child, query);
                if (item) return item;
            }
        }
    }

    return nullptr;
}

void ObjectFinder::findAllItems(QObject *root, const QJsonObject &query, QObjectList &results, QSet<QObject*> &visited)
{
    if (!root) return;

    // Skip already visited objects
    if (visited.contains(root)) return;
    visited.insert(root);

    if (matchObject(root, query)) {
        results.append(root);
    }

    // Traverse QObject children
    QList<QObject*> children = root->children();
    for (QObject *child : children) {
        findAllItems(child, query, results, visited);
    }

    // Traverse QQuickItem visual children
    if (isQuickItem(root)) {
        QQuickItem *quickItem = toQuickItem(root);
        QList<QQuickItem*> childItems = quickItem->childItems();
        for (QQuickItem *child : childItems) {
            findAllItems(child, query, results, visited);
        }
    }
}

bool ObjectFinder::matchObject(QObject *object, const QJsonObject &query)
{
    if (query.contains("objID")) {
        if (getObjID(object) != query["objID"].toString()) {
            return false;
        }
    }

    if (query.contains("objectName")) {
        if (object->objectName() != query["objectName"].toString()) {
            return false;
        }
    }

    if (query.contains("className")) {
        if (getClassName(object) != query["className"].toString()) {
            return false;
        }
    }

    if (query.contains("properties")) {
        if (!matchProperties(object, query["properties"].toObject())) {
            return false;
        }
    }

    if (query.contains("path")) {
        if (!matchPath(object, query["path"].toString())) {
            return false;
        }
    }

    if (query.contains("visible") && isQuickItem(object)) {
        QQuickItem *item = toQuickItem(object);
        if (item->isVisible() != query["visible"].toBool()) {
            return false;
        }
    }

    if (query.contains("enabled") && isQuickItem(object)) {
        QQuickItem *item = toQuickItem(object);
        if (item->isEnabled() != query["enabled"].toBool()) {
            return false;
        }
    }

    return true;
}

bool ObjectFinder::matchProperties(QObject *object, const QJsonObject &properties)
{
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        QString propName = it.key();
        QJsonValue propValue = it.value();

        QVariant actualValue = object->property(propName.toUtf8().constData());

        if (!actualValue.isValid()) {
            return false;
        }

        if (propValue.isString()) {
            if (actualValue.toString() != propValue.toString()) return false;
        } else if (propValue.isBool()) {
            if (actualValue.toBool() != propValue.toBool()) return false;
        } else if (propValue.isDouble()) {
            if (actualValue.toDouble() != propValue.toDouble()) return false;
        }
    }

    return true;
}

bool ObjectFinder::matchPath(QObject *object, const QString &path)
{
    QStringList pathParts = path.split('/');
    QStringList actualParts = getObjectPathList(object);

    int pathIdx = pathParts.size() - 1;
    int actualIdx = actualParts.size() - 1;

    while (pathIdx >= 0 && actualIdx >= 0) {
        if (pathParts[pathIdx] != actualParts[actualIdx]) {
            return false;
        }
        pathIdx--;
        actualIdx--;
    }

    return pathIdx < 0;
}

QJsonObject ObjectFinder::getObjectTree()
{
    QList<QObject*> roots = m_engine->rootObjects();

    if (roots.isEmpty()) {
        return QJsonObject();
    }

    QJsonObject rootNode;
    rootNode["objectName"] = "root";
    rootNode["className"] = "Root";

    QJsonArray children;
    for (QObject *root : roots) {
        children.append(buildObjectTree(root, 0));
    }
    rootNode["children"] = children;

    return rootNode;
}

QJsonObject ObjectFinder::buildObjectTree(QObject *object, int depth)
{
    QJsonObject node;

    node["objID"] = getObjID(object);
    node["objectName"] = object->objectName();
    node["className"] = getClassName(object);

    if (isQuickItem(object)) {
        QQuickItem *item = toQuickItem(object);
        node["visible"] = item->isVisible();
        node["enabled"] = item->isEnabled();
        node["geometry"] = JsonHelper::geometryJson(
            item->x(), item->y(), item->width(), item->height()
        );
    }

    QJsonArray children;
    QList<QObject*> objChildren = object->children();
    for (QObject *child : objChildren) {
        children.append(buildObjectTree(child, depth + 1));
    }

    if (isQuickItem(object)) {
        QQuickItem *item = toQuickItem(object);
        QList<QQuickItem*> itemChildren = item->childItems();
        for (QQuickItem *child : itemChildren) {
            if (!objChildren.contains(child)) {
                children.append(buildObjectTree(child, depth + 1));
            }
        }
    }

    if (!children.isEmpty()) {
        node["children"] = children;
    }

    return node;
}

QJsonObject ObjectFinder::getObjectInfo(QObject *object)
{
    QJsonObject info;

    info["objID"] = getObjID(object);
    info["objectName"] = object->objectName();
    info["className"] = getClassName(object);
    info["path"] = getObjectPath(object);

    if (isQuickItem(object)) {
        QQuickItem *item = toQuickItem(object);
        info["visible"] = item->isVisible();
        info["enabled"] = item->isEnabled();
        info["geometry"] = JsonHelper::geometryJson(
            item->x(), item->y(), item->width(), item->height()
        );
    } else {
        info["visible"] = true;
        info["enabled"] = true;
    }

    info["properties"] = getObjectProperties(object);

    return info;
}

QJsonObject ObjectFinder::getObjectProperties(QObject *object)
{
    QJsonObject properties;

    const QMetaObject *meta = object->metaObject();
    for (int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty prop = meta->property(i);
        QString propName = prop.name();

        if (propName == "objectName" || propName == "parent" ||
            propName == "children" || propName == "className") {
            continue;
        }

        QVariant propValue = prop.read(object);
        properties[propName] = JsonHelper::variantToJson(propValue);
    }

    QString objID = getObjID(object);
    if (!objID.isEmpty()) {
        properties["objID"] = objID;
    }

    return properties;
}

QJsonObject ObjectFinder::getAppInfo()
{
    QJsonObject info;
    info["appName"] = QCoreApplication::applicationName();
    info["version"] = QCoreApplication::applicationVersion();
    info["qtVersion"] = qVersion();
    info["objectCount"] = getObjectCount();

    QList<QObject*> roots = m_engine->rootObjects();
    for (QObject *root : roots) {
        QQuickWindow *window = qobject_cast<QQuickWindow*>(root);
        if (window) {
            QJsonObject windowInfo;
            windowInfo["title"] = window->title();
            windowInfo["width"] = window->width();
            windowInfo["height"] = window->height();
            info["mainWindow"] = windowInfo;
            break;
        }
    }

    return info;
}

int ObjectFinder::getObjectCount()
{
    int count = 0;

    std::function<void(QObject*)> countObjects = [&](QObject *obj) {
        if (!obj) return;
        count++;

        for (QObject *child : obj->children()) {
            countObjects(child);
        }

        if (isQuickItem(obj)) {
            QQuickItem *item = toQuickItem(obj);
            for (QQuickItem *child : item->childItems()) {
                if (!obj->children().contains(child)) {
                    countObjects(child);
                }
            }
        }
    };

    QList<QObject*> roots = m_engine->rootObjects();
    for (QObject *root : roots) {
        countObjects(root);
    }

    return count;
}

QStringList ObjectFinder::getObjectPathList(QObject *object)
{
    QStringList path;

    QObject *current = object;
    while (current) {
        QString name = current->objectName();
        if (name.isEmpty()) {
            name = getClassName(current);
        }
        path.prepend(name);
        current = current->parent();
    }

    return path;
}

QString ObjectFinder::getObjectPath(QObject *object)
{
    return getObjectPathList(object).join('/');
}

bool ObjectFinder::isQuickItem(QObject *object)
{
    return qobject_cast<QQuickItem*>(object) != nullptr;
}

QQuickItem *ObjectFinder::toQuickItem(QObject *object)
{
    return qobject_cast<QQuickItem*>(object);
}

QString ObjectFinder::getClassName(QObject *object)
{
    return QString::fromUtf8(object->metaObject()->className());
}

QString ObjectFinder::getObjID(QObject *object)
{
    return object->property("objID").toString();
}
