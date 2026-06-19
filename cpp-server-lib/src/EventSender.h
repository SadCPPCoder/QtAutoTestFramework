#ifndef EVENTSENDER_H
#define EVENTSENDER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QQuickWindow>
#include <QQuickItem>
#include <QMutex>

class EventSender : public QObject
{
    Q_OBJECT

public:
    explicit EventSender(QObject *parent = nullptr);

    void setTargetWindow(QQuickWindow *window);

    bool sendMouseEvent(QObject *target, const QJsonObject &event);
    bool sendKeyboardEvent(const QJsonObject &event);
    bool sendTouchEvent(const QJsonObject &event);
    bool sendWheelEvent(int x, int y, int deltaX, int deltaY);

private slots:
    void doMouseClick(int x, int y, int button, int modifiers);
    void doMouseDoubleClick(int x, int y, int button);
    void doMousePress(int x, int y, int button);
    void doMouseRelease(int x, int y, int button);
    void doMouseMove(int x, int y);
    void doKeyPress(int key, int modifiers);
    void doKeyRelease(int key, int modifiers);
    void doKeyClick(int key, int modifiers);
    void doKeyType(const QString &text);
    void doKeySequence(const QString &sequence);
    void doTouchTap(int x, int y);
    void doWheelEvent(int x, int y, int deltaX, int deltaY);

private:
    Qt::MouseButton parseMouseButton(const QString &button);
    Qt::KeyboardModifiers parseModifiers(const QJsonArray &modifiers);
    int parseKeyCode(const QString &key);

    QQuickWindow *m_window = nullptr;
    QMutex m_mutex;
    bool m_result = false;
};

#endif // EVENTSENDER_H
