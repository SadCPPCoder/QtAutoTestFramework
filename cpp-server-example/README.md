# QtAutoTestFramework - 示例应用

这是一个使用 QtAutoTestServer 库的示例 QML 应用程序。

## 功能

- 简单的登录界面（用户名/密码）
- 所有 UI 控件都设置了 `objectName` 和 `objID` 属性
- 支持通过命令行参数启用测试服务

## 构建

### 前置条件

- Qt 6.5+
- CMake 3.16+
- MSVC 2019+

### 构建步骤

```powershell
cd cpp-server-example
mkdir build
cd build
cmake .. -G "Visual Studio 18 2022" -A x64
cmake --build . --config Release
```

或使用 Qt Creator 打开 `CMakeLists.txt`。

## 运行

```powershell
# 普通运行
./DemoApp.exe

# 启用测试服务
./DemoApp.exe --test-server

# 指定端口和日志
./DemoApp.exe --test-server --test-port=9090 --test-log-level=DEBUG
```

## 命令行参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `--test-server` | 启用测试服务 | 不启用 |
| `--test-port` | 服务端口 | 8080 |
| `--test-log-level` | 日志级别 | INFO |
| `--test-log-file` | 日志文件路径 | test_server.log |

## QML 对象命名

所有可测试的 UI 控件都设置了以下属性：

| 控件 | objectName | objID |
|------|------------|-------|
| 主窗口 | mainWindow | main_window_001 |
| 用户名输入框 | usernameField | input_username_001 |
| 密码输入框 | passwordField | input_password_001 |
| 记住我复选框 | rememberCheckbox | checkbox_remember_001 |
| 登录按钮 | loginButton | btn_login_001 |
| 状态标签 | statusLabel | label_status_001 |
| 登录成功对话框 | loginSuccessDialog | dialog_login_success_001 |
| 登录失败对话框 | loginFailedDialog | dialog_login_failed_001 |

## 测试验证

启动应用后，使用 Python 客户端进行测试：

```python
from qtautotest import QtTestClient

with QtTestClient("localhost", 8080) as client:
    # 获取应用信息
    info = client.get_app_info()
    print(f"App: {info['appName']}")

    # 通过 objID 查找登录按钮
    btn = client.get_object("btn_login_001")
    print(f"Button: {btn.object_name}")

    # 点击登录
    client.click(obj_id="btn_login_001")
```
