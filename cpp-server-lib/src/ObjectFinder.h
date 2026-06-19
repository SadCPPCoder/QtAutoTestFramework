#ifndef OBJECTFINDER_H
#define OBJECTFINDER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QSet>

class ObjectFinder : public QObject
{
    Q_OBJECT

public:
    explicit ObjectFinder(QQmlApplicationEngine *engine, QObject *parent = nullptr);

    QJsonArray findObjects(const QJsonObject &query);
    QJsonObject findObject(const QJsonObject &query);
    QJsonObject findById(const QString &objID);
    QQuickItem *findQuickItem(const QJsonObject &query);

    QJsonObject getObjectTree();
    QJsonObject getObjectInfo(QObject *object);
    QJsonObject getObjectProperties(QObject *object);

    QJsonObject getAppInfo();
    int getObjectCount();

private:
    void findAllItems(QObject *root, const QJsonObject &query, QObjectList &results, QSet<QObject*> &visited);
    QQuickItem *findFirstItem(QObject *root, const QJsonObject &query);
    bool matchObject(QObject *object, const QJsonObject &query);
    bool matchProperties(QObject *object, const QJsonObject &properties);
    bool matchPath(QObject *object, const QString &path);

    QJsonObject buildObjectTree(QObject *object, int depth = 0);
    QString getObjectPath(QObject *object);
    QStringList getObjectPathList(QObject *object);

    bool isQuickItem(QObject *object);
    QQuickItem *toQuickItem(QObject *object);
    QString getClassName(QObject *object);
    QString getObjID(QObject *object);

    QQmlApplicationEngine *m_engine;
};

#endif // OBJECTFINDER_H
