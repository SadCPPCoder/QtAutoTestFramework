#ifndef _WIN32

#include "HttpServer.h"
#include "Logger.h"
#include <microhttpd.h>
#include <QMutex>
#include <QWaitCondition>
#include <QHash>
#include <QDateTime>
#include <cstring>

// Request context for tracking individual requests
struct RequestContext
{
    QByteArray body;
    quint64 requestId;
    bool headersParsed = false;
};

// Pending response waiting for main thread to process
struct PendingResponse
{
    QMutex mutex;
    QWaitCondition condition;
    int statusCode = 0;
    QByteArray body;
    bool ready = false;
};

class HttpServer::Impl
{
public:
    struct MHD_Daemon *daemon = nullptr;

    // Map from requestId to pending response
    QMutex responseMutex;
    QHash<quint64, PendingResponse*> pendingResponses;

    // Request counter for generating unique IDs
    QAtomicInteger<quint64> requestCounter{0};

    quint64 generateRequestId()
    {
        return requestCounter.fetchAndAddRelaxed(1);
    }

    static MHD_Result handleRequest(void *cls,
                             struct MHD_Connection *connection,
                             const char *url,
                             const char *method,
                             const char *version,
                             const char *uploadData,
                             size_t *uploadDataSize,
                             void **conCls)
    {
        Q_UNUSED(version);

        HttpServer *server = static_cast<HttpServer*>(cls);
        Impl *impl = server->m_impl;

        // First call for this connection - initialize context
        if (*conCls == nullptr) {
            auto *ctx = new RequestContext();
            ctx->requestId = impl->generateRequestId();
            *conCls = ctx;
            return MHD_YES;
        }

        auto *ctx = static_cast<RequestContext*>(*conCls);

        // Accumulate body data
        if (*uploadDataSize > 0) {
            ctx->body.append(uploadData, *uploadDataSize);
            *uploadDataSize = 0;
            return MHD_YES;
        }

        // Request is complete - process it
        QString qMethod = QString::fromUtf8(method);
        QString qUrl = QString::fromUtf8(url);

        // Remove query string from path
        int queryIndex = qUrl.indexOf('?');
        QString path = (queryIndex >= 0) ? qUrl.left(queryIndex) : qUrl;

        LOG_INFO(QString("%1 %2 (body: %3 bytes)")
                 .arg(qMethod)
                 .arg(path)
                 .arg(ctx->body.size()));

        // Create pending response
        auto *pending = new PendingResponse();
        {
            QMutexLocker locker(&impl->responseMutex);
            impl->pendingResponses[ctx->requestId] = pending;
        }

        // Emit signal to main thread and wait for response
        emit server->requestReceived(ctx->requestId, qMethod, path, ctx->body);

        // Wait for main thread to provide the response
        {
            QMutexLocker locker(&pending->mutex);
            while (!pending->ready) {
                pending->condition.wait(&pending->mutex);
            }
        }

        // Send the response
        struct MHD_Response *response = MHD_create_response_from_buffer(
            pending->body.size(),
            (void*)pending->body.constData(),
            MHD_RESPMEM_MUST_COPY
        );

        MHD_add_response_header(response, "Content-Type",
                                "application/json; charset=utf-8");
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");

        MHD_Result ret = MHD_queue_response(connection, pending->statusCode, response);
        MHD_destroy_response(response);

        // Cleanup
        {
            QMutexLocker locker(&impl->responseMutex);
            impl->pendingResponses.remove(ctx->requestId);
        }
        delete pending;
        delete ctx;
        *conCls = nullptr;

        return ret;
    }
};

HttpServer::HttpServer(QObject *parent)
    : QThread(parent)
    , m_impl(new Impl())
{
}

HttpServer::~HttpServer()
{
    stop();
    delete m_impl;
}

bool HttpServer::startServer(quint16 port)
{
    m_port = port;

    // Start MHD daemon with internal polling thread
    m_impl->daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD,
        port,
        nullptr, nullptr,
        &Impl::handleRequest, this,
        MHD_OPTION_END
    );

    if (!m_impl->daemon) {
        LOG_ERROR("Failed to start MHD daemon");
        return false;
    }

    m_running = true;
    LOG_INFO(QString("HTTP Server started on port %1 (Linux/libmicrohttpd)").arg(port));

    return true;
}

void HttpServer::stop()
{
    m_running = false;

    if (m_impl->daemon) {
        MHD_stop_daemon(m_impl->daemon);
        m_impl->daemon = nullptr;
    }

    // Wake up any pending responses
    {
        QMutexLocker locker(&m_impl->responseMutex);
        for (auto it = m_impl->pendingResponses.begin();
             it != m_impl->pendingResponses.end(); ++it) {
            PendingResponse *pending = it.value();
            QMutexLocker respLocker(&pending->mutex);
            pending->ready = true;
            pending->statusCode = 503;
            pending->body = "{\"error\":{\"code\":\"SERVER_STOPPING\",\"message\":\"Server is stopping\"}}";
            pending->condition.wakeAll();
        }
        m_impl->pendingResponses.clear();
    }

    LOG_INFO("HTTP Server stopped");
}

void HttpServer::run()
{
    // MHD handles its own threads with MHD_USE_SELECT_INTERNALLY
    // This thread just needs to stay alive
    while (m_running) {
        QThread::msleep(100);
    }
}

void HttpServer::sendResponse(quint64 requestId, int statusCode, const QByteArray &body)
{
    QMutexLocker locker(&m_impl->responseMutex);

    PendingResponse *pending = m_impl->pendingResponses.value(requestId);
    if (pending) {
        QMutexLocker respLocker(&pending->mutex);
        pending->statusCode = statusCode;
        pending->body = body;
        pending->ready = true;
        pending->condition.wakeOne();
    } else {
        LOG_WARNING(QString("No pending response for request %1").arg(requestId));
    }
}

void HttpServer::sendJsonResponse(quint64 requestId, int statusCode, const QByteArray &jsonBody)
{
    sendResponse(requestId, statusCode, jsonBody);
}

#endif // !_WIN32
