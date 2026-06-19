#include "qtautotest/QtAutoTestServer.h"
#include "HttpServer.h"
#include "RequestHandler.h"
#include "Logger.h"

QtAutoTestServer::QtAutoTestServer(QQmlApplicationEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_httpServer(nullptr)
    , m_requestHandler(nullptr)
{
}

QtAutoTestServer::~QtAutoTestServer()
{
    stop();
}

bool QtAutoTestServer::start(quint16 port)
{
    // Configure logger
    Logger *logger = Logger::instance();
    logger->setLogFile(m_logFile);

    if (m_logLevel == "DEBUG") {
        logger->setLogLevel(Logger::Debug);
    } else if (m_logLevel == "WARNING") {
        logger->setLogLevel(Logger::Warning);
    } else if (m_logLevel == "ERROR") {
        logger->setLogLevel(Logger::Error);
    } else {
        logger->setLogLevel(Logger::Info);
    }

    LOG_INFO("========================================");
    LOG_INFO("QtAutoTestServer Starting...");
    LOG_INFO(QString("Version: %1").arg(QTAT_VERSION_STRING));
    LOG_INFO(QString("Port: %1").arg(port));
    LOG_INFO(QString("Log Level: %1").arg(m_logLevel));
    LOG_INFO(QString("Log File: %1").arg(m_logFile));
    LOG_INFO("========================================");

    m_port = port;

    // Create request handler
    m_requestHandler = new RequestHandler(m_engine, this);

    // Create HTTP server
    m_httpServer = new HttpServer(this);

    // Connect signals
    connect(m_httpServer, &HttpServer::requestReceived,
            m_requestHandler, &RequestHandler::handleRequest);

    m_requestHandler->setHttpServer(m_httpServer);

    // Start server
    if (!m_httpServer->startServer(m_port)) {
        LOG_ERROR("Failed to start HTTP server");
        return false;
    }

    m_running = true;
    LOG_INFO("QtAutoTestServer started successfully");
    return true;
}

void QtAutoTestServer::stop()
{
    if (!m_running) {
        return;
    }

    LOG_INFO("Stopping QtAutoTestServer...");

    if (m_httpServer) {
        m_httpServer->stop();
        delete m_httpServer;
        m_httpServer = nullptr;
    }

    if (m_requestHandler) {
        delete m_requestHandler;
        m_requestHandler = nullptr;
    }

    m_running = false;
    LOG_INFO("QtAutoTestServer stopped");
}

void QtAutoTestServer::updateWindow()
{
    LOG_INFO("Updating window reference...");

    QList<QObject*> roots = m_engine->rootObjects();
    for (QObject *root : roots) {
        QQuickWindow *window = qobject_cast<QQuickWindow*>(root);
        if (window) {
            LOG_INFO(QString("Found window: %1").arg(window->title()));
            if (m_requestHandler) {
                m_requestHandler->updateWindow(window);
            }
            break;
        }
    }
}

void QtAutoTestServer::setLogLevel(const QString &level)
{
    m_logLevel = level;
}

void QtAutoTestServer::setLogFile(const QString &path)
{
    m_logFile = path;
}

bool QtAutoTestServer::isRunning() const
{
    return m_running;
}
