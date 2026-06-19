/**
 * @file main.cpp
 * @brief QtAutoTestFramework 示例应用入口
 *
 * 演示如何在 Qt QML 应用中集成 QtAutoTestServer。
 * 通过命令行参数 --test-server 启用测试服务。
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QDebug>
#include <qtautotest/QtAutoTestServer.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("QtAutoTestDemo");
    QCoreApplication::setApplicationVersion("1.0.0");

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt AutoTest Framework Demo Application");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption testServerOption("test-server", "Enable the automated test server");
    parser.addOption(testServerOption);

    QCommandLineOption testPortOption(
        QStringList() << "test-port",
        "Port for the test server (default: 8080)",
        "port", "8080"
    );
    parser.addOption(testPortOption);

    QCommandLineOption logLevelOption(
        QStringList() << "test-log-level",
        "Log level: DEBUG, INFO, WARNING, ERROR (default: INFO)",
        "level", "INFO"
    );
    parser.addOption(logLevelOption);

    QCommandLineOption logFileOption(
        QStringList() << "test-log-file",
        "Log file path (default: test_server.log)",
        "path", "test_server.log"
    );
    parser.addOption(logFileOption);

    parser.process(app);

    // Create QML engine
    QQmlApplicationEngine engine;

    // Create test server (always create, but only start if requested)
    QtAutoTestServer testServer(&engine);

    if (parser.isSet(testServerOption)) {
        testServer.setLogLevel(parser.value(logLevelOption));
        testServer.setLogFile(parser.value(logFileOption));

        if (!testServer.start(parser.value(testPortOption).toUShort())) {
            qCritical() << "Failed to start test server";
        }
    }

    // Load QML
    qDebug() << "Loading QML...";
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load QML";
        if (testServer.isRunning()) {
            qDebug() << "Test server is running on port" << parser.value(testPortOption).toUShort();
            return app.exec();
        }
        return -1;
    }

    qDebug() << "QML loaded successfully";

    // Update window reference after QML is loaded
    testServer.updateWindow();

    return app.exec();
}
