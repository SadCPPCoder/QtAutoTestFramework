#include "ScreenCapture.h"
#include "Logger.h"
#include <QBuffer>
#include <QImage>
#include <QScreen>
#include <QGuiApplication>

ScreenCapture::ScreenCapture(QObject *parent)
    : QObject(parent)
{
}

void ScreenCapture::setTargetWindow(QQuickWindow *window)
{
    m_window = window;
}

QJsonObject ScreenCapture::captureWindow()
{
    if (!m_window) {
        LOG_ERROR("Target window not set");
        return QJsonObject();
    }

    QImage image = grabWindow();
    if (image.isNull()) {
        LOG_ERROR("Failed to grab window");
        return QJsonObject();
    }

    return imageToJson(image);
}

QJsonObject ScreenCapture::captureObject(QQuickItem *item)
{
    if (!item) {
        LOG_ERROR("Target item is null");
        return QJsonObject();
    }

    QImage image = grabItem(item);
    if (image.isNull()) {
        LOG_ERROR("Failed to grab item");
        return QJsonObject();
    }

    return imageToJson(image);
}

QJsonObject ScreenCapture::captureRegion(int x, int y, int width, int height)
{
    if (!m_window) {
        LOG_ERROR("Target window not set");
        return QJsonObject();
    }

    QImage fullImage = grabWindow();
    if (fullImage.isNull()) {
        return QJsonObject();
    }

    QImage regionImage = fullImage.copy(x, y, width, height);
    return imageToJson(regionImage);
}

QImage ScreenCapture::grabWindow()
{
    if (!m_window) {
        return QImage();
    }

    QImage image = m_window->grabWindow();
    if (!image.isNull()) {
        return image;
    }

    QScreen *screen = m_window->screen();
    if (screen) {
        WId winId = m_window->winId();
        QPixmap pixmap = screen->grabWindow(winId);
        if (!pixmap.isNull()) {
            return pixmap.toImage();
        }
    }

    LOG_ERROR("All screenshot methods failed");
    return QImage();
}

QImage ScreenCapture::grabItem(QQuickItem *item)
{
    if (!item) {
        return QImage();
    }

    QQuickWindow *window = item->window();
    if (!window) {
        return QImage();
    }

    QImage fullImage = grabWindow();
    if (fullImage.isNull()) {
        return QImage();
    }

    QPointF scenePos = item->mapToScene(QPointF(0, 0));
    QRectF itemRect(scenePos.x(), scenePos.y(), item->width(), item->height());

    return fullImage.copy(itemRect.toRect());
}

QJsonObject ScreenCapture::imageToJson(const QImage &image)
{
    if (image.isNull()) {
        return QJsonObject();
    }

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    QJsonObject result;
    result["image"] = QString::fromLatin1(ba.toBase64());
    result["width"] = image.width();
    result["height"] = image.height();
    result["format"] = "png";

    return result;
}
