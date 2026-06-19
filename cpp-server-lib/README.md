# QtAutoTestServer - C++ 框架库

Qt QML 应用程序自动化测试框架的服务端库。通过嵌入 HTTP 服务器，为 QML 应用提供 REST API，支持远程对象查询、事件发送和截图功能。

## 功能

- **对象查询**：通过 objectName、类型、属性、路径等条件查找 QML 对象
- **事件发送**：发送鼠标、键盘、触摸、滚轮事件到 QML 控件
- **截图功能**：截取整个窗口或指定控件的截图
- **异步等待**：等待控件出现/消失
- **详细日志**：记录所有 API 调用和事件发送

## 系统要求

- Windows 10/11
- Qt 6.5+
- CMake 3.16+
- MSVC 2019+

## 构建

```powershell
cd cpp-server-lib
mkdir build
cd build
cmake .. -G "Visual Studio 18 2022" -A x64
cmake --build . --config Release
```

## 集成指南

### CMake 集成

```cmake
# 方式1：作为子目录
add_subdirectory(path/to/cpp-server-lib)
target_link_libraries(YourApp PRIVATE QtAutoTestServer)

# 方式2：预编译库
find_package(QtAutoTestServer REQUIRED)
target_link_libraries(YourApp PRIVATE QtAutoTestServer)
```

### qmake 集成

```pro
# 添加头文件路径
INCLUDEPATH += path/to/cpp-server-lib/inc

# 链接库
LIBS += -Lpath/to/cpp-server-lib/build -lQtAutoTestServer
LIBS += -lHttpapi
```

## 使用方法

### 基本集成

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <qtautotest/QtAutoTestServer.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // 创建测试服务器
    QtAutoTestServer testServer(&engine);

    // 解析命令行参数
    QCommandLineParser parser;
    parser.addOption(QCommandLineOption("test-server", "Enable test server"));
    parser.addOption(QCommandLineOption("test-port", "Port", "port", "8080"));
    parser.process(app);

    // 启动测试服务
    if (parser.isSet("test-server")) {
        testServer.setLogLevel("INFO");
        testServer.start(parser.value("test-port").toUShort());
    }

    // 加载 QML
    engine.load(QUrl("qrc:/main.qml"));

    // 更新窗口引用（必须在 QML 加载后调用）
    testServer.updateWindow();

    return app.exec();
}
```

### 命令行参数

```bash
# 启用测试服务
YourApp.exe --test-server

# 指定端口
YourApp.exe --test-server --test-port=9090

# 完整配置
YourApp.exe --test-server --test-port=8080 --test-log-level=DEBUG --test-log-file=debug.log
```

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `--test-server` | 启用测试服务 | 不启用 |
| `--test-port` | 服务端口 | 8080 |
| `--test-log-level` | 日志级别 | INFO |
| `--test-log-file` | 日志文件路径 | test_server.log |

## API 参考

### QtAutoTestServer 类

```cpp
class QtAutoTestServer : public QObject
{
public:
    // 构造函数
    explicit QtAutoTestServer(QQmlApplicationEngine *engine, QObject *parent = nullptr);

    // 启动测试服务
    bool start(quint16 port = 8080);

    // 停止测试服务
    void stop();

    // 更新窗口引用（QML 加载后调用）
    void updateWindow();

    // 设置日志级别: "DEBUG", "INFO", "WARNING", "ERROR"
    void setLogLevel(const QString &level);

    // 设置日志文件路径
    void setLogFile(const QString &path);

    // 检查是否运行中
    bool isRunning() const;
};
```

## REST API

### 基础信息

- **基础 URL**: `http://localhost:{port}/api/v1`
- **Content-Type**: `application/json`
- **字符编码**: UTF-8

### API 端点

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/health` | 健康检查 |
| GET | `/app/info` | 应用信息 |
| GET | `/objects/tree` | 对象树 |
| POST | `/objects/query` | 查询对象 |
| GET | `/objects/{id}` | 对象详情 |
| GET | `/objects/{id}/properties` | 对象属性 |
| POST | `/events` | 发送事件 |
| GET | `/screenshot` | 截图 |
| POST | `/wait` | 等待条件 |

### 查询对象

```json
POST /objects/query
{
  "objectName": "loginButton",
  "className": "QQuickButton",
  "properties": {"text": "Login"},
  "visible": true
}
```

### 发送事件

```json
POST /events
{
  "target": {"objID": "btn_login_001"},
  "event": {
    "type": "mouse",
    "action": "click",
    "button": "left",
    "x": 60,
    "y": 20
  }
}
```

### 事件类型

| 类型 | 动作 | 参数 |
|------|------|------|
| mouse | click, doubleClick, press, release, move | button, x, y, modifiers |
| keyboard | press, release, click, type, sequence | key, text, modifiers |
| touch | tap | x, y |
| wheel | scroll | x, y, deltaX, deltaY |

## QML 对象命名

在 QML 中为需要测试的控件设置 `objectName` 和 `objID` 属性：

```qml
Button {
    objectName: "loginButton"
    property string objID: "btn_login_001"
    text: "Login"
}

TextField {
    objectName: "usernameField"
    property string objID: "input_username_001"
    placeholderText: "Username"
}
```

## 线程模型

```
Main Thread (GUI)
├── QML Rendering
├── EventSender (事件执行)
├── ObjectFinder (对象查找)
└── ScreenCapture (截图)

HTTP Server Thread
└── HttpServer (请求接收)
    └── 信号 → RequestHandler (主线程)
```

## 日志

```cpp
// 设置日志级别
testServer.setLogLevel("DEBUG");

// 设置日志文件
testServer.setLogFile("test_server.log");
```

日志输出示例：
```
[2026-06-19 15:03:09.843] [INFO] QtAutoTestServer Starting...
[2026-06-19 15:03:09.844] [INFO] Port: 8080
[2026-06-19 15:03:09.844] [INFO] HTTP Server started on port 8080
[2026-06-19 15:03:09.844] [INFO] POST /api/v1/events (body: 80 bytes)
[2026-06-19 15:03:09.844] [INFO] Executing mouse click at (400, 300)
```

## 故障排除

### 端口被占用

```
HttpAddUrl failed: 183
```

**解决**：更换端口或关闭占用端口的程序。

### 权限不足

```
HttpAddUrl failed: 5
```

**解决**：以管理员身份运行，或使用 netsh 保留 URL：

```bash
netsh http add urlacl url=http://+:8080/ user=Everyone
```

### 事件没有效果

**解决**：
1. 确保 QML 已正确加载
2. 确保调用了 `updateWindow()`
3. 重启应用

### 截图失败

**解决**：offscreen 模式下截图可能不可用，这是正常现象。
