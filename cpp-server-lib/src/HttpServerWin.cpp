#ifdef _WIN32

#include "HttpServer.h"
#include "Logger.h"
#include <windows.h>
#include <http.h>

class HttpServer::Impl
{
public:
    HANDLE m_requestQueue = INVALID_HANDLE_VALUE;

    QString parseMethod(HTTP_VERB verb)
    {
        switch (verb) {
            case HttpVerbGET:     return "GET";
            case HttpVerbPOST:    return "POST";
            case HttpVerbPUT:     return "PUT";
            case HttpVerbDELETE:  return "DELETE";
            case HttpVerbHEAD:    return "HEAD";
            case HttpVerbOPTIONS: return "OPTIONS";
            default:              return "UNKNOWN";
        }
    }

    QString parseUrl(const HTTP_COOKED_URL &cookedUrl)
    {
        QString fullPath = QString::fromWCharArray(
            cookedUrl.pAbsPath,
            cookedUrl.AbsPathLength / sizeof(WCHAR)
        );

        int queryIndex = fullPath.indexOf('?');
        if (queryIndex >= 0) {
            fullPath = fullPath.left(queryIndex);
        }

        return fullPath;
    }

    QByteArray receiveBody(HANDLE requestQueue, HTTP_REQUEST_ID requestId)
    {
        QByteArray body;
        QByteArray bodyBuffer(64 * 1024, 0);

        while (true) {
            ULONG bytesRead = 0;
            ULONG result = HttpReceiveRequestEntityBody(
                requestQueue,
                requestId,
                0,
                bodyBuffer.data(),
                bodyBuffer.size(),
                &bytesRead,
                nullptr
            );

            if (result == NO_ERROR && bytesRead > 0) {
                body.append(bodyBuffer.data(), bytesRead);
            } else {
                break;
            }
        }

        return body;
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
    HTTPAPI_VERSION version = HTTPAPI_VERSION_1;
    ULONG result = HttpInitialize(version, HTTP_INITIALIZE_SERVER, nullptr);
    if (result != NO_ERROR) {
        LOG_ERROR(QString("HttpInitialize failed: %1").arg(result));
        return false;
    }

    result = HttpCreateHttpHandle(&m_impl->m_requestQueue, 0);
    if (result != NO_ERROR) {
        LOG_ERROR(QString("HttpCreateHttpHandle failed: %1").arg(result));
        HttpTerminate(HTTP_INITIALIZE_SERVER, nullptr);
        return false;
    }

    m_port = port;
    QString url = QString("http://+:%1/").arg(port);
    result = HttpAddUrl(m_impl->m_requestQueue, (PCWSTR)url.utf16(), nullptr);
    if (result != NO_ERROR) {
        LOG_ERROR(QString("HttpAddUrl failed: %1").arg(result));
        CloseHandle(m_impl->m_requestQueue);
        HttpTerminate(HTTP_INITIALIZE_SERVER, nullptr);
        return false;
    }

    m_running = true;
    QThread::start();

    LOG_INFO(QString("HTTP Server started on port %1 (Windows)").arg(port));
    return true;
}

void HttpServer::stop()
{
    m_running = false;

    if (m_impl->m_requestQueue != INVALID_HANDLE_VALUE) {
        CloseHandle(m_impl->m_requestQueue);
        m_impl->m_requestQueue = INVALID_HANDLE_VALUE;
    }

    wait();

    QString url = QString("http://+:%1/").arg(m_port);
    HttpRemoveUrl(nullptr, (PCWSTR)url.utf16());
    HttpTerminate(HTTP_INITIALIZE_SERVER, nullptr);

    LOG_INFO("HTTP Server stopped");
}

void HttpServer::run()
{
    QByteArray buffer(64 * 1024, 0);
    PHTTP_REQUEST request = (PHTTP_REQUEST)buffer.data();

    while (m_running) {
        ULONG bytesRead = 0;
        ULONG result = HttpReceiveHttpRequest(
            m_impl->m_requestQueue,
            HTTP_NULL_ID,
            0,
            request,
            buffer.size(),
            &bytesRead,
            nullptr
        );

        if (result == NO_ERROR) {
            QString method = m_impl->parseMethod(request->Verb);
            QString path = m_impl->parseUrl(request->CookedUrl);
            quint64 requestId = request->RequestId;

            QByteArray body;
            if (method == "POST" || method == "PUT") {
                body = m_impl->receiveBody(m_impl->m_requestQueue, requestId);
            }

            LOG_INFO(QString("%1 %2 (body: %3 bytes)").arg(method).arg(path).arg(body.size()));

            emit requestReceived(requestId, method, path, body);

        } else if (result == ERROR_MORE_DATA) {
            buffer.resize(bytesRead);
            request = (PHTTP_REQUEST)buffer.data();
        } else {
            if (m_running) {
                LOG_WARNING(QString("HttpReceiveHttpRequest failed: %1").arg(result));
            }
            break;
        }
    }
}

void HttpServer::sendResponse(quint64 requestId, int statusCode, const QByteArray &body)
{
    HTTP_RESPONSE response;
    ZeroMemory(&response, sizeof(response));

    response.StatusCode = (USHORT)statusCode;
    response.pReason = statusCode == 200 ? "OK" : "Error";
    response.ReasonLength = (USHORT)strlen(response.pReason);

    response.Headers.KnownHeaders[HttpHeaderContentType].pRawValue = "application/json; charset=utf-8";
    response.Headers.KnownHeaders[HttpHeaderContentType].RawValueLength = 31;

    HTTP_DATA_CHUNK dataChunk;
    dataChunk.DataChunkType = HttpDataChunkFromMemory;
    dataChunk.FromMemory.pBuffer = (PVOID)body.constData();
    dataChunk.FromMemory.BufferLength = (ULONG)body.size();
    response.EntityChunkCount = 1;
    response.pEntityChunks = &dataChunk;

    ULONG bytesSent = 0;
    HttpSendHttpResponse(
        m_impl->m_requestQueue,
        requestId,
        0,
        &response,
        nullptr,
        &bytesSent,
        nullptr,
        0,
        nullptr,
        nullptr
    );
}

void HttpServer::sendJsonResponse(quint64 requestId, int statusCode, const QByteArray &jsonBody)
{
    sendResponse(requestId, statusCode, jsonBody);
}

#endif // _WIN32
