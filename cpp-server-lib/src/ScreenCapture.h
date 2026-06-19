#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <QObject>
#include <QJsonObject>
#include <QQuickWindow>
#include <QQuickItem>

class ScreenCapture : public QObject
{
    Q_OBJECT

public:
    explicit ScreenCapture(QObject *parent = nullptr);

    void setTargetWindow(QQuickWindow *window);

    QJsonObject captureWindow();
    QJsonObject captureObject(QQuickItem *item);
    QJsonObject captureRegion(int x, int y, int width, int height);

private:
    QImage grabWindow();
    QImage grabItem(QQuickItem *item);
    QJsonObject imageToJson(const QImage &image);

    QQuickWindow *m_window = nullptr;
};

#endif // SCREENCAPTURE_H
