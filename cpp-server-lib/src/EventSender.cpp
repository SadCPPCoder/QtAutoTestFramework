#include "EventSender.h"
#include "Logger.h"
#include <QtTest/QTest>
#include <QCoreApplication>
#include <QThread>
#include <QWheelEvent>

EventSender::EventSender(QObject *parent)
    : QObject(parent)
{
}

void EventSender::setTargetWindow(QQuickWindow *window)
{
    QMutexLocker locker(&m_mutex);
    m_window = window;
    LOG_INFO(QString("EventSender: Window set to %1").arg(window ? "valid" : "null"));
}

Qt::MouseButton EventSender::parseMouseButton(const QString &button)
{
    QString lower = button.toLower();
    if (lower == "left") return Qt::LeftButton;
    if (lower == "right") return Qt::RightButton;
    if (lower == "middle") return Qt::MiddleButton;
    return Qt::LeftButton;
}

Qt::KeyboardModifiers EventSender::parseModifiers(const QJsonArray &modifiers)
{
    Qt::KeyboardModifiers result = Qt::NoModifier;
    for (const QJsonValue &mod : modifiers) {
        QString lower = mod.toString().toLower();
        if (lower == "ctrl") result |= Qt::ControlModifier;
        else if (lower == "alt") result |= Qt::AltModifier;
        else if (lower == "shift") result |= Qt::ShiftModifier;
        else if (lower == "meta" || lower == "win") result |= Qt::MetaModifier;
    }
    return result;
}

int EventSender::parseKeyCode(const QString &key)
{
    QString lower = key.toLower();

    if (lower == "enter" || lower == "return") return Qt::Key_Return;
    if (lower == "tab") return Qt::Key_Tab;
    if (lower == "escape" || lower == "esc") return Qt::Key_Escape;
    if (lower == "backspace") return Qt::Key_Backspace;
    if (lower == "delete" || lower == "del") return Qt::Key_Delete;
    if (lower == "space") return Qt::Key_Space;
    if (lower == "up") return Qt::Key_Up;
    if (lower == "down") return Qt::Key_Down;
    if (lower == "left") return Qt::Key_Left;
    if (lower == "right") return Qt::Key_Right;
    if (lower == "home") return Qt::Key_Home;
    if (lower == "end") return Qt::Key_End;
    if (lower == "pageup") return Qt::Key_PageUp;
    if (lower == "pagedown") return Qt::Key_PageDown;

    if (lower.startsWith("f")) {
        bool ok;
        int num = lower.mid(1).toInt(&ok);
        if (ok && num >= 1 && num <= 12) {
            return Qt::Key_F1 + num - 1;
        }
    }

    if (lower.length() == 1) {
        QChar ch = lower[0];
        if (ch.isLetter()) return Qt::Key_A + ch.toLatin1() - 'a';
        if (ch.isDigit()) return Qt::Key_0 + ch.toLatin1() - '0';
    }

    return Qt::Key_unknown;
}

// ============ Main thread slots ============

void EventSender::doMouseClick(int x, int y, int button, int modifiers)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing mouse click at (%1, %2)").arg(x).arg(y));
    QTest::mouseClick(m_window, static_cast<Qt::MouseButton>(button),
                      static_cast<Qt::KeyboardModifiers>(modifiers), QPoint(x, y));
    QCoreApplication::processEvents();
    QThread::msleep(50);  // Give QML time to process
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doMouseDoubleClick(int x, int y, int button)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing mouse double click at (%1, %2)").arg(x).arg(y));
    QTest::mouseDClick(m_window, static_cast<Qt::MouseButton>(button),
                       Qt::NoModifier, QPoint(x, y));
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doMousePress(int x, int y, int button)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing mouse press at (%1, %2)").arg(x).arg(y));
    QTest::mousePress(m_window, static_cast<Qt::MouseButton>(button),
                      Qt::NoModifier, QPoint(x, y));
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doMouseRelease(int x, int y, int button)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing mouse release at (%1, %2)").arg(x).arg(y));
    QTest::mouseRelease(m_window, static_cast<Qt::MouseButton>(button),
                        Qt::NoModifier, QPoint(x, y));
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doMouseMove(int x, int y)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing mouse move to (%1, %2)").arg(x).arg(y));
    QTest::mouseMove(m_window, QPoint(x, y));
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doKeyPress(int key, int modifiers)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing key press: %1").arg(key));
    QTest::keyPress(m_window, static_cast<Qt::Key>(key),
                    static_cast<Qt::KeyboardModifiers>(modifiers));
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doKeyRelease(int key, int modifiers)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing key release: %1").arg(key));
    QTest::keyRelease(m_window, static_cast<Qt::Key>(key),
                      static_cast<Qt::KeyboardModifiers>(modifiers));
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doKeyClick(int key, int modifiers)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing key click: %1").arg(key));
    QTest::keyClick(m_window, static_cast<Qt::Key>(key),
                    static_cast<Qt::KeyboardModifiers>(modifiers));
    QCoreApplication::processEvents();
    QThread::msleep(50);
    m_result = true;
}

void EventSender::doKeyType(const QString &text)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing key type: %1").arg(text));

    for (const QChar &ch : text) {
        int key = ch.unicode();
        if (ch.isUpper() || QString("~!@#$%^&*()_+{}|:\"<>?").contains(ch)) {
            QTest::keyClick(m_window, key, Qt::ShiftModifier);
        } else {
            QTest::keyClick(m_window, key);
        }
        QCoreApplication::processEvents();
        QThread::msleep(30);
    }
    QCoreApplication::processEvents();
    QThread::msleep(50);
    m_result = true;
}

void EventSender::doKeySequence(const QString &sequence)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing key sequence: %1").arg(sequence));

    QStringList parts = sequence.split('+');
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    int key = 0;

    for (const QString &part : parts) {
        QString trimmed = part.trimmed().toLower();
        if (trimmed == "ctrl") modifiers |= Qt::ControlModifier;
        else if (trimmed == "alt") modifiers |= Qt::AltModifier;
        else if (trimmed == "shift") modifiers |= Qt::ShiftModifier;
        else if (trimmed == "meta" || trimmed == "win") modifiers |= Qt::MetaModifier;
        else key = parseKeyCode(trimmed);
    }

    if (key == 0) { m_result = false; return; }

    QTest::keyClick(m_window, static_cast<Qt::Key>(key), modifiers);
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doTouchTap(int x, int y)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing touch tap at (%1, %2)").arg(x).arg(y));
    QTest::mouseClick(m_window, Qt::LeftButton, Qt::NoModifier, QPoint(x, y));
    QCoreApplication::processEvents();
    m_result = true;
}

void EventSender::doWheelEvent(int x, int y, int deltaX, int deltaY)
{
    if (!m_window) { m_result = false; return; }
    LOG_INFO(QString("Executing wheel at (%1, %2)").arg(x).arg(y));

    QPoint pos(x, y);
    QPoint angleDelta(deltaX, deltaY);
    QWheelEvent event(QPointF(pos), QPointF(pos), QPoint(), angleDelta,
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(m_window, &event);
    QCoreApplication::processEvents();
    m_result = true;
}

// ============ Thread-safe public interface ============

bool EventSender::sendMouseEvent(QObject *target, const QJsonObject &event)
{
    QString action = event["action"].toString();
    int x = event["x"].toInt();
    int y = event["y"].toInt();
    int button = parseMouseButton(event["button"].toString("left"));
    int modifiers = parseModifiers(event["modifiers"].toArray());

    QQuickItem *item = qobject_cast<QQuickItem*>(target);
    if (item) {
        QPointF scenePos = item->mapToScene(QPointF(x, y));
        x = scenePos.x();
        y = scenePos.y();
    }

    LOG_INFO(QString("Dispatching mouse event: %1 at (%2, %3)").arg(action).arg(x).arg(y));

    QThread *mainThread = QCoreApplication::instance()->thread();
    bool isMainThread = (QThread::currentThread() == mainThread);

    if (isMainThread) {
        if (action == "click") doMouseClick(x, y, button, modifiers);
        else if (action == "doubleClick") doMouseDoubleClick(x, y, button);
        else if (action == "press") doMousePress(x, y, button);
        else if (action == "release") doMouseRelease(x, y, button);
        else if (action == "move") doMouseMove(x, y);
        else { LOG_WARNING("Unknown mouse action: " + action); return false; }
    } else {
        if (action == "click") {
            QMetaObject::invokeMethod(this, "doMouseClick", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, x), Q_ARG(int, y),
                                      Q_ARG(int, button), Q_ARG(int, modifiers));
        } else if (action == "doubleClick") {
            QMetaObject::invokeMethod(this, "doMouseDoubleClick", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, button));
        } else if (action == "press") {
            QMetaObject::invokeMethod(this, "doMousePress", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, button));
        } else if (action == "release") {
            QMetaObject::invokeMethod(this, "doMouseRelease", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, x), Q_ARG(int, y), Q_ARG(int, button));
        } else if (action == "move") {
            QMetaObject::invokeMethod(this, "doMouseMove", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, x), Q_ARG(int, y));
        } else {
            LOG_WARNING("Unknown mouse action: " + action);
            return false;
        }
    }

    return m_result;
}

bool EventSender::sendKeyboardEvent(const QJsonObject &event)
{
    QString action = event["action"].toString();
    LOG_INFO(QString("Dispatching keyboard event: %1").arg(action));

    QThread *mainThread = QCoreApplication::instance()->thread();
    bool isMainThread = (QThread::currentThread() == mainThread);

    if (isMainThread) {
        if (action == "press") {
            doKeyPress(parseKeyCode(event["key"].toString()), parseModifiers(event["modifiers"].toArray()));
        } else if (action == "release") {
            doKeyRelease(parseKeyCode(event["key"].toString()), parseModifiers(event["modifiers"].toArray()));
        } else if (action == "click") {
            doKeyClick(parseKeyCode(event["key"].toString()), parseModifiers(event["modifiers"].toArray()));
        } else if (action == "type") {
            doKeyType(event["text"].toString());
        } else if (action == "sequence") {
            doKeySequence(event["sequence"].toString());
        } else {
            LOG_WARNING("Unknown keyboard action: " + action);
            return false;
        }
    } else {
        if (action == "press") {
            int key = parseKeyCode(event["key"].toString());
            int mods = parseModifiers(event["modifiers"].toArray());
            QMetaObject::invokeMethod(this, "doKeyPress", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, key), Q_ARG(int, mods));
        } else if (action == "release") {
            int key = parseKeyCode(event["key"].toString());
            int mods = parseModifiers(event["modifiers"].toArray());
            QMetaObject::invokeMethod(this, "doKeyRelease", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, key), Q_ARG(int, mods));
        } else if (action == "click") {
            int key = parseKeyCode(event["key"].toString());
            int mods = parseModifiers(event["modifiers"].toArray());
            QMetaObject::invokeMethod(this, "doKeyClick", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, key), Q_ARG(int, mods));
        } else if (action == "type") {
            QString text = event["text"].toString();
            QMetaObject::invokeMethod(this, "doKeyType", Qt::BlockingQueuedConnection,
                                      Q_ARG(QString, text));
        } else if (action == "sequence") {
            QString sequence = event["sequence"].toString();
            QMetaObject::invokeMethod(this, "doKeySequence", Qt::BlockingQueuedConnection,
                                      Q_ARG(QString, sequence));
        } else {
            LOG_WARNING("Unknown keyboard action: " + action);
            return false;
        }
    }

    return m_result;
}

bool EventSender::sendTouchEvent(const QJsonObject &event)
{
    QString action = event["action"].toString();
    int x = event["x"].toInt();
    int y = event["y"].toInt();

    LOG_INFO(QString("Dispatching touch event: %1 at (%2, %3)").arg(action).arg(x).arg(y));

    QThread *mainThread = QCoreApplication::instance()->thread();
    bool isMainThread = (QThread::currentThread() == mainThread);

    if (isMainThread) {
        doTouchTap(x, y);
    } else {
        QMetaObject::invokeMethod(this, "doTouchTap", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, x), Q_ARG(int, y));
    }

    return m_result;
}

bool EventSender::sendWheelEvent(int x, int y, int deltaX, int deltaY)
{
    LOG_INFO(QString("Dispatching wheel event at (%1, %2)").arg(x).arg(y));

    QThread *mainThread = QCoreApplication::instance()->thread();
    bool isMainThread = (QThread::currentThread() == mainThread);

    if (isMainThread) {
        doWheelEvent(x, y, deltaX, deltaY);
    } else {
        QMetaObject::invokeMethod(this, "doWheelEvent", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, x), Q_ARG(int, y),
                                  Q_ARG(int, deltaX), Q_ARG(int, deltaY));
    }

    return m_result;
}
