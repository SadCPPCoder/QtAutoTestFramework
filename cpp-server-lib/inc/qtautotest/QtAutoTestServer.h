#ifndef QTAUTOTESTSERVER_H
#define QTAUTOTESTSERVER_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "qtat_export.h"
#include "qtat_config.h"

// Forward declarations for internal classes
class HttpServer;
class RequestHandler;

/**
 * @brief Qt 自动化测试服务端主类
 *
 * 这是框架库的唯一公共入口类。用户创建此类的实例并传入
 * QQmlApplicationEngine 指针，即可为 QML 应用启用 HTTP 测试服务。
 *
 * 使用示例:
 * @code
 *   QQmlApplicationEngine engine;
 *   QtAutoTestServer testServer(&engine);
 *   testServer.start(8080);
 *
 *   engine.load(QUrl("qrc:/main.qml"));
 *   testServer.updateWindow();  // QML 加载后调用
 *   app.exec();
 * @endcode
 *
 * 启动后，测试服务监听 HTTP 端口，提供以下 REST API:
 * - GET  /api/v1/health          - 健康检查
 * - GET  /api/v1/app/info        - 应用信息
 * - GET  /api/v1/objects/tree    - 对象树
 * - POST /api/v1/objects/query   - 查询对象
 * - GET  /api/v1/objects/{id}    - 对象详情
 * - POST /api/v1/events          - 发送事件
 * - GET  /api/v1/screenshot      - 截图
 * - POST /api/v1/wait            - 等待条件
 */
class QTAUTOTEST_EXPORT QtAutoTestServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param engine QML 应用引擎指针，用于访问 QML 对象树
     * @param parent 父对象
     */
    explicit QtAutoTestServer(QQmlApplicationEngine *engine, QObject *parent = nullptr);

    ~QtAutoTestServer();

    /**
     * @brief 启动测试服务
     * @param port 监听端口号，默认 8080
     * @return 成功返回 true，失败返回 false
     *
     * 启动 HTTP 服务器监听指定端口。需要在 QML 加载之前调用。
     * 服务运行在独立线程中，不会阻塞 UI 线程。
     */
    bool start(quint16 port = 8080);

    /**
     * @brief 停止测试服务
     *
     * 停止 HTTP 服务器并释放资源。通常在应用退出时自动调用。
     */
    void stop();

    /**
     * @brief 更新窗口引用
     *
     * 在 QML 加载完成后调用，将 QQuickWindow 引用传递给事件发送器
     * 和截图模块。必须在 engine.load() 之后调用。
     */
    void updateWindow();

    /**
     * @brief 设置日志级别
     * @param level 日志级别: "DEBUG", "INFO", "WARNING", "ERROR"
     */
    void setLogLevel(const QString &level);

    /**
     * @brief 设置日志文件路径
     * @param path 日志文件路径，默认 "test_server.log"
     */
    void setLogFile(const QString &path);

    /**
     * @brief 检查服务是否正在运行
     * @return 运行中返回 true
     */
    bool isRunning() const;

private:
    QQmlApplicationEngine *m_engine;
    HttpServer *m_httpServer;
    RequestHandler *m_requestHandler;

    quint16 m_port = 8080;
    QString m_logLevel = "INFO";
    QString m_logFile = "test_server.log";
    bool m_running = false;
};

#endif // QTAUTOTESTSERVER_H
