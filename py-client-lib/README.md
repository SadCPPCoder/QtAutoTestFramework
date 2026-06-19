# qtautotest - Python Client Library

Python 客户端库，用于与集成了 QtAutoTestServer 的 Qt 应用进行交互。

## 安装

```bash
cd py-client-lib
pip install -e .
```

如果需要异步客户端支持：

```bash
pip install -e ".[async]"
```

## 快速开始

### 同步客户端

```python
from qtautotest import QtTestClient

# 使用上下文管理器
with QtTestClient("localhost", 8080) as client:
    # 获取应用信息
    info = client.get_app_info()
    print(f"App: {info['appName']}")

    # 通过 objID 查找并点击按钮
    btn = client.get_object("btn_login_001")
    client.click(obj_id=btn.obj_id)

    # 输入文本
    client.key_type("Hello World")

    # 截图
    client.screenshot().save("result.png")

    # 断言
    client.assert_visible(obj_id="btn_login_001")
```

### 异步客户端

```python
from qtautotest import AsyncQtTestClient
import asyncio

async def test():
    async with AsyncQtTestClient("localhost", 8080) as client:
        # 通过 objID 查找并点击按钮
        btn = await client.get_object("btn_login_001")
        await client.click(obj_id=btn.obj_id)

        await client.key_type("Hello")

        screenshot = await client.screenshot()
        screenshot.save("result.png")

asyncio.run(test())
```

## API 参考

### 对象查询

```python
# 查找单个对象
obj = client.find_object(objectName="loginButton")
obj = client.find_object(objID="btn_login_001")
obj = client.find_object(className="QQuickButton", visible=True)

# 查找多个对象
buttons = client.find_objects(className="QQuickButton")

# 通过 ID 获取
obj = client.get_object("btn_login_001")

# 获取属性
props = client.get_properties("btn_login_001")

# 获取对象树
tree = client.get_object_tree()
tree.print_tree()

# 获取应用信息
info = client.get_app_info()
```

### 鼠标事件

```python
# 点击
client.click(obj_id="btn_login_001")
client.click(x=100, y=200)
client.click(obj_id="btn_login_001", button="right")

# 双击
client.double_click(obj_id="input_field")

# 右键点击
client.right_click(x=400, y=300)
```

### 键盘事件

```python
# 输入文本
client.key_type("Hello World")

# 按键
client.key_click("enter")
client.key_click("tab")

# 快捷键
client.key_sequence("Ctrl+S")
client.key_sequence("Ctrl+Shift+A")
```

### 触摸和滚轮

```python
# 触摸
client.tap(x=100, y=200)

# 滚动
client.scroll(x=400, y=300, delta_y=120)
```

### 截图

```python
# 截取整个窗口
screenshot = client.screenshot()
screenshot.save("window.png")
screenshot.show()  # 显示图片

# 截取指定对象
screenshot = client.screenshot(obj_id="btn_login_001")
screenshot.save("button.png")
```

### 等待

```python
# 等待
client.wait(1000)  # 等待 1 秒

# 等待对象出现
obj = client.wait_for_object(
    objectName="dialog",
    timeout=5000,
    interval=100
)

# 等待对象消失
client.wait_for_object_disappear(
    objectName="loading_spinner",
    timeout=10000
)
```

### 断言

```python
# 存在性
client.assert_exists(objectName="loginButton")
client.assert_not_exists(objectName="nonExistent")

# 可见性
client.assert_visible(obj_id="btn_login_001")

# 文本
client.assert_text(obj_id="label_title", text="Welcome")

# 启用状态
client.assert_enabled(obj_id="btn_submit")
```

### 配置

```python
# 设置操作延迟
client.set_delay(0.1)

# 设置重试参数
client.set_retry(count=3, interval=1.0)

# 设置默认超时
client.set_default_timeout(5000)
```

## 数据模型

### QObjectInfo

```python
@dataclass
class QObjectInfo:
    obj_id: str           # 对象唯一 ID
    object_name: str      # objectName
    class_name: str       # QML 类型名
    properties: dict      # 属性字典
    geometry: QRect       # 位置和大小
    visible: bool         # 是否可见
    enabled: bool         # 是否可用
    path: str             # 对象树路径
```

### QRect

```python
@dataclass
class QRect:
    x: int
    y: int
    width: int
    height: int

    @property
    def center(self) -> tuple  # 中心点坐标
```

### QScreenshot

```python
class QScreenshot:
    image_data: bytes     # 图片数据
    width: int
    height: int

    def save(path: str)   # 保存到文件
    def show()            # 显示图片
    def to_pil() -> Image # 转为 PIL Image
    def to_base64() -> str
```

### QObjectTree

```python
class QObjectTree:
    obj_id: str                   # 对象唯一 ID
    object_name: str              # objectName
    class_name: str               # QML 类型名
    properties: dict              # 属性字典
    children: List[QObjectTree]   # 子节点列表

    def find(**kwargs) -> Optional[QObjectTree]     # 查找第一个匹配节点
    def find_all(**kwargs) -> List[QObjectTree]     # 查找所有匹配节点
    def print_tree(indent: int = 0) -> None         # 打印对象树
    def to_dict() -> dict                           # 转为字典
```

**使用示例**：

```python
tree = client.get_object_tree()
tree.print_tree()  # 打印完整树

# 在树中查找
node = tree.find(objectName="loginButton")
if node:
    print(f"Found: {node.object_name}")

# 查找所有按钮
buttons = tree.find_all(className="QQuickButton")
```

## 异常

| 异常 | 说明 |
|------|------|
| `ConnectionError` | 连接失败 |
| `ObjectNotFoundError` | 对象未找到 |
| `MultipleObjectsFoundError` | 找到多个对象 |
| `TimeoutError` | 操作超时 |
| `EventError` | 事件发送失败 |
| `AssertionError_` | 断言失败 |
| `ServerError` | 服务器错误 |
