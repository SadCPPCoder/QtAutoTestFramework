# QtAutoTestFramework

Qt QML 应用程序自动化测试框架，提供 C++ 服务端库和 Python 客户端库，支持通过 HTTP REST API 对 Qt 应用进行 UI 自动化测试。

**跨平台支持：Windows / Linux**

## 架构概览

```
┌─────────────────────────────────────────────────────────────────┐
│                      Python Test Client                          │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  qtautotest.QtTestClient                                  │  │
│  │    find_object()  click()  key_type()  screenshot()       │  │
│  │    wait_for_object()  assert_exists()  assert_text()      │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                                │
                                │ HTTP REST API (JSON)
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Qt Application                              │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  QML UI (QQuickWindow)                                    │  │
│  │    Button { objectName: "btn"; property string objID }    │  │
│  └───────────────────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  QtAutoTestServer (cpp-server-lib)                        │  │
│  │    ┌──────────────┐ ┌──────────────┐ ┌──────────────┐    │  │
│  │    │  HttpServer  │ │ObjectFinder  │ │ EventSender  │    │  │
│  │    │  (跨平台)    │ │              │ │              │    │  │
│  │    └──────────────┘ └──────────────┘ └──────────────┘    │  │
│  │    ┌──────────────┐ ┌──────────────┐ ┌──────────────┐    │  │
│  │    │ScreenCapture │ │    Logger    │ │  JsonHelper  │    │  │
│  │    └──────────────┘ └──────────────┘ └──────────────┘    │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## 平台支持

| 平台 | HTTP 服务器实现 | 编译器 |
|------|----------------|--------|
| Windows 10/11 | Windows HTTP Server API | MSVC 2019+ |
| Linux (Ubuntu, Fedora, Arch等) | libmicrohttpd | GCC 11+ / Clang 14+ |

## 目录结构

```
QtAutoTestFramework/
├── cpp-server-lib/          # C++ 框架库（静态库）
│   ├── inc/qtautotest/      # 公共头文件
│   └── src/                 # 内部实现
│       ├── HttpServer.h     # 平台无关接口
│       ├── HttpServerWin.cpp   # Windows 实现
│       └── HttpServerLinux.cpp # Linux 实现
├── cpp-server-example/      # 示例 QML 应用
│   ├── src/main.cpp
│   └── qml/main.qml
├── py-client-lib/           # Python 客户端库
│   └── qtautotest/          # 包源码
└── py-client-example/       # Python 测试示例
    ├── test_health.py
    └── test_login.py
```

## 快速开始

### 1. 在 Qt 应用中集成测试服务

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qtautotest/QtAutoTestServer.h>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // 创建测试服务
    QtAutoTestServer testServer(&engine);

    // 通过命令行参数启用
    if (app.arguments().contains("--test-server")) {
        testServer.start(8080);
    }

    engine.load(QUrl("qrc:/main.qml"));
    testServer.updateWindow();
    return app.exec();
}
```

### 2. 在 QML 中标记测试对象

```qml
Button {
    objectName: "loginButton"
    property string objID: "btn_login_001"
    text: "Login"
}
```

### 3. 使用 Python 进行测试

```python
from qtautotest import QtTestClient

with QtTestClient("localhost", 8080) as client:
    # 查找并点击按钮
    client.click(obj_id="btn_login_001")

    # 输入文本
    client.key_type("Hello World")

    # 截图
    client.screenshot().save("result.png")

    # 断言
    client.assert_visible(obj_id="btn_login_001")
```

## 系统要求

### Windows
| 组件 | 要求 |
|------|------|
| 操作系统 | Windows 10/11 |
| Qt | 6.5+ |
| 编译器 | MSVC 2019+ |
| Python | 3.8+ |

### Linux
| 组件 | 要求 |
|------|------|
| 操作系统 | Ubuntu 20.04+ / Fedora 36+ / Arch Linux |
| Qt | 6.5+ |
| 编译器 | GCC 11+ / Clang 14+ |
| 依赖库 | libmicrohttpd-dev |
| Python | 3.8+ |

## 构建

### Windows

```powershell
# C++ 库
cd cpp-server-lib
mkdir build && cd build
cmake .. -G "Visual Studio 18 2022" -A x64
cmake --build . --config Release

# 示例应用
cd ../../cpp-server-example
mkdir build && cd build
cmake .. -G "Visual Studio 18 2022" -A x64
cmake --build . --config Release
```

### Linux

```bash
# 安装依赖
sudo apt-get install -y libmicrohttpd-dev pkg-config

# C++ 库
cd cpp-server-lib
mkdir build && cd build
cmake ..
make -j$(nproc)

# 示例应用
cd ../../cpp-server-example
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Python 库

```bash
cd py-client-lib
pip install -e .
```

## API 文档

- [C++ 服务端库文档](cpp-server-lib/README.md)
- [示例应用文档](cpp-server-example/README.md)
- [Python 客户端库文档](py-client-lib/README.md)
- [Python 测试示例文档](py-client-example/README.md)

## 跨平台测试

### 本地测试

```python
from qtautotest import QtTestClient

with QtTestClient("localhost", 8080) as client:
    client.click(obj_id="btn_login_001")
    client.key_type("Hello")
    client.screenshot().save("result.png")
```

### 远程测试（从 Windows 测试 Linux 应用）

```python
from qtautotest import QtTestClient

# 连接到远程 Linux 服务器
with QtTestClient("192.168.1.100", 8080) as client:
    client.click(obj_id="btn_login_001")
    client.key_type("Hello from Windows")
    client.screenshot().save("remote_result.png")
```

### 测试脚本

```bash
# 运行基础测试
cd py-client-example
python test_health.py

# 运行登录测试
python test_login.py

# 运行可视化测试（带延迟，可观察UI变化）
python test_observe.py
```

## 许可证

MIT License - 详见 [LICENSE](LICENSE)
