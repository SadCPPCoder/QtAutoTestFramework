# Qt AutoTest - Python 测试示例

使用 `qtautotest` 客户端库编写的自动化测试脚本。

## 前置条件

1. 已安装 Python 3.8+
2. 已安装 `qtautotest` 库
3. Qt 应用已启动并启用了测试服务

## 安装依赖

```bash
cd py-client-example
pip install -r requirements.txt
```

或直接安装客户端库：

```bash
cd ../py-client-lib
pip install -e .
```

## 测试脚本

### test_health.py - 健康检查测试

测试基础 API 功能：健康检查、应用信息、对象树、对象查询、截图。

```bash
python test_health.py
```

**测试内容：**
- 健康检查 API
- 获取应用信息
- 获取对象树
- 查询对象（按类型、名称、ID）
- 截图功能

### test_login.py - 登录测试

测试登录流程，包含清理功能。

```bash
python test_login.py
```

**测试内容：**
- 成功登录测试
- 登录失败测试
- 自动清理输入框和对话框

## 启动测试应用

```powershell
# 进入示例应用目录
cd ../cpp-server-example/build/Release

# 启动应用（启用测试服务）
.\DemoApp.exe --test-server --test-port=8080
```

## 运行测试

```bash
# 运行健康检查测试
python test_health.py

# 运行登录测试
python test_login.py
```

## 测试输出示例

```
============================================================
Qt AutoTest - Health Check Tests
============================================================

=== Test Health Check ===
  App Name: QtAutoTestDemo
  Qt Version: 6.5.3
  Object Count: 15
[OK] Health check passed

=== Test Object Tree ===
Object tree structure:
Root: root
  ApplicationWindow: mainWindow [main_window_001]
    QQuickContentItem: ApplicationWindow
      ColumnLayout:
        Label:
        GroupBox: User Information
          ...
[OK] Object tree retrieved

=== Test Object Query ===
1. Querying all buttons...
   Found 1 buttons
   - loginButton: Login

2. Querying by objectName...
   Found: loginButton (QQuickButton)

3. Querying by objID...
   Found: loginButton (btn_login_001)
[OK] Object query test passed

=== Test Screenshot ===
  Screenshot saved: 800x600
[OK] Screenshot test passed

============================================================
[OK] All tests passed!
============================================================
```

## 截图保存

测试截图保存在 `screenshots/` 目录：

```
screenshots/
├── health_check.png
├── login_success.png
└── login_failure.png
```

## 故障排除

### 连接失败

```
ConnectionError: Failed to connect to http://localhost:8080/api/v1
```

**解决：** 确保 Qt 应用已启动并使用了 `--test-server` 参数。

### 对象未找到

```
ObjectNotFoundError: Object not found: {'objectName': 'xxx'}
```

**解决：** 检查 QML 中是否设置了 `objectName` 或 `objID` 属性。

### 截图失败

截图返回 None，可能是因为 offscreen 模式。

## 编写自定义测试

```python
from qtautotest import QtTestClient

with QtTestClient("localhost", 8080) as client:
    # 通过 objID 查找对象（推荐）
    button = client.get_object("btn_login_001")
    
    # 点击
    client.click(obj_id=button.obj_id)
    
    # 输入文本
    client.key_type("Hello")
    
    # 等待对象出现
    dialog = client.wait_for_object(objectName="dialog", timeout=5000)
    
    # 断言
    client.assert_visible(obj_id="dialog_001")
    
    # 截图
    client.screenshot().save("result.png")
```
