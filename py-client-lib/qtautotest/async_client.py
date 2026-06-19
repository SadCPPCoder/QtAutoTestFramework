"""
Qt Test Client - Asynchronous client for Qt AutoTest Framework
"""

import asyncio
import aiohttp
import base64
import logging
from typing import Dict, Any, List, Optional

from .models import QRect, QObjectInfo, QScreenshot, QObjectTree
from .exceptions import (
    ConnectionError,
    ObjectNotFoundError,
    MultipleObjectsFoundError,
    TimeoutError,
    ServerError,
)

logger = logging.getLogger("qtautotest")


class AsyncQtTestClient:
    """
    Asynchronous client for Qt AutoTest Framework.

    Usage:
        async with AsyncQtTestClient("localhost", 8080) as client:
            await client.click(objectName="button")
            await client.key_type("Hello")
            screenshot = await client.screenshot()
            screenshot.save("test.png")
    """

    def __init__(self, host: str = "localhost", port: int = 8080,
                 timeout: int = 30, delay: float = 0.1):
        """
        Initialize the async client.

        Args:
            host: Server hostname
            port: Server port
            timeout: Request timeout in seconds
            delay: Delay between operations in seconds
        """
        self.base_url = f"http://{host}:{port}/api/v1"
        self.timeout = aiohttp.ClientTimeout(total=timeout)
        self.delay = delay
        self.session: Optional[aiohttp.ClientSession] = None

    async def connect(self) -> None:
        """Connect to the server"""
        self.session = aiohttp.ClientSession(timeout=self.timeout)
        try:
            async with self.session.get(f"{self.base_url}/health") as response:
                response.raise_for_status()
                logger.info(f"Connected to {self.base_url}")
        except Exception as e:
            await self.disconnect()
            raise ConnectionError(f"Failed to connect: {e}")

    async def disconnect(self) -> None:
        """Disconnect from the server"""
        if self.session:
            await self.session.close()
            self.session = None
        logger.info("Disconnected")

    async def __aenter__(self):
        await self.connect()
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        await self.disconnect()
        return False

    # ============ Object Query ============

    async def find_object(self, **kwargs) -> Optional[QObjectInfo]:
        """Find a single object matching the query."""
        objects = await self.find_objects(**kwargs)
        if not objects:
            return None
        if len(objects) > 1:
            from .exceptions import MultipleObjectsFoundError
            raise MultipleObjectsFoundError(query=kwargs, count=len(objects))
        return objects[0]

    async def find_objects(self, **kwargs) -> List[QObjectInfo]:
        """Find all objects matching the query."""
        response = await self._post("/objects/query", kwargs)

        if response.status in (400, 404):
            return []

        response.raise_for_status()
        data = await response.json()

        return [QObjectInfo.from_dict(obj) for obj in data.get("objects", [])]

    async def get_object(self, obj_id: str) -> Optional[QObjectInfo]:
        """Get object by objID."""
        try:
            response = await self._get(f"/objects/{obj_id}")
            if response.status == 404:
                return None
            response.raise_for_status()
            data = await response.json()
            return QObjectInfo.from_dict(data)
        except Exception:
            return None

    async def get_properties(self, obj_id: str) -> Dict[str, Any]:
        """Get all properties of an object."""
        response = await self._get(f"/objects/{obj_id}/properties")
        response.raise_for_status()
        data = await response.json()
        return data.get("properties", {})

    async def get_object_tree(self) -> QObjectTree:
        """Get the complete QML object tree."""
        response = await self._get("/objects/tree")
        response.raise_for_status()
        data = await response.json()
        return QObjectTree.from_dict(data)

    async def get_app_info(self) -> Dict[str, Any]:
        """Get application information."""
        response = await self._get("/app/info")
        response.raise_for_status()
        return await response.json()

    # ============ Mouse Events ============

    async def click(self, obj_id: str = None, x: int = None, y: int = None,
                    button: str = "left", modifiers: List[str] = None) -> bool:
        """Click on an object or at coordinates."""
        target = {}
        if obj_id:
            target["objID"] = obj_id

        event = {
            "type": "mouse",
            "action": "click",
            "button": button,
            "x": x or 0,
            "y": y or 0,
            "modifiers": modifiers or []
        }

        return await self._send_event(target, event)

    async def double_click(self, obj_id: str = None, x: int = None,
                           y: int = None) -> bool:
        """Double click on an object or at coordinates."""
        target = {}
        if obj_id:
            target["objID"] = obj_id

        event = {
            "type": "mouse",
            "action": "doubleClick",
            "button": "left",
            "x": x or 0,
            "y": y or 0
        }

        return await self._send_event(target, event)

    async def right_click(self, obj_id: str = None, x: int = None,
                          y: int = None) -> bool:
        """Right click on an object or at coordinates."""
        target = {}
        if obj_id:
            target["objID"] = obj_id

        event = {
            "type": "mouse",
            "action": "click",
            "button": "right",
            "x": x or 0,
            "y": y or 0
        }

        return await self._send_event(target, event)

    # ============ Keyboard Events ============

    async def key_type(self, text: str) -> bool:
        """Type text character by character."""
        event = {
            "type": "keyboard",
            "action": "type",
            "text": text
        }
        return await self._send_event({}, event)

    async def key_click(self, key: str, modifiers: List[str] = None) -> bool:
        """Press and release a key."""
        event = {
            "type": "keyboard",
            "action": "click",
            "key": key,
            "modifiers": modifiers or []
        }
        return await self._send_event({}, event)

    async def key_sequence(self, sequence: str) -> bool:
        """Execute a key sequence like 'Ctrl+S'."""
        event = {
            "type": "keyboard",
            "action": "sequence",
            "sequence": sequence
        }
        return await self._send_event({}, event)

    # ============ Touch Events ============

    async def tap(self, x: int, y: int) -> bool:
        """Tap at coordinates."""
        event = {
            "type": "touch",
            "action": "tap",
            "x": x,
            "y": y
        }
        return await self._send_event({}, event)

    # ============ Wheel Events ============

    async def scroll(self, x: int, y: int, delta_x: int = 0,
                     delta_y: int = 0) -> bool:
        """Scroll at coordinates."""
        event = {
            "type": "wheel",
            "action": "scroll",
            "x": x,
            "y": y,
            "deltaX": delta_x,
            "deltaY": delta_y
        }
        return await self._send_event({}, event)

    # ============ Screenshot ============

    async def screenshot(self, obj_id: str = None) -> Optional[QScreenshot]:
        """Take a screenshot."""
        params = {}
        if obj_id:
            params["objID"] = obj_id

        try:
            response = await self._get("/screenshot", params=params)

            if response.status >= 400:
                return None

            data = await response.json()

            if "image" not in data:
                return None

            image_data = base64.b64decode(data["image"])
            return QScreenshot(
                image_data=image_data,
                width=data.get("width", 0),
                height=data.get("height", 0)
            )
        except Exception:
            return None

    # ============ Wait ============

    async def wait(self, milliseconds: int) -> None:
        """Wait for specified milliseconds."""
        await asyncio.sleep(milliseconds / 1000)

    async def wait_for_object(self, timeout: int = 5000, interval: int = 100,
                              **kwargs) -> Optional[QObjectInfo]:
        """Wait for an object to appear."""
        start_time = asyncio.get_event_loop().time()
        timeout_sec = timeout / 1000

        while asyncio.get_event_loop().time() - start_time < timeout_sec:
            obj = await self.find_object(**kwargs)
            if obj:
                return obj
            await asyncio.sleep(interval / 1000)

        return None

    async def wait_for_object_disappear(self, timeout: int = 5000,
                                        interval: int = 100,
                                        **kwargs) -> bool:
        """Wait for an object to disappear."""
        start_time = asyncio.get_event_loop().time()
        timeout_sec = timeout / 1000

        while asyncio.get_event_loop().time() - start_time < timeout_sec:
            obj = await self.find_object(**kwargs)
            if not obj:
                return True
            await asyncio.sleep(interval / 1000)

        return False

    # ============ Assertions ============

    async def assert_exists(self, **kwargs) -> None:
        """Assert that an object exists."""
        obj = await self.find_object(**kwargs)
        if not obj:
            from .exceptions import AssertionError_
            raise AssertionError_(f"Object not found: {kwargs}")

    async def assert_not_exists(self, **kwargs) -> None:
        """Assert that an object does not exist."""
        obj = await self.find_object(**kwargs)
        if obj:
            from .exceptions import AssertionError_
            raise AssertionError_(f"Object found: {kwargs}")

    async def assert_visible(self, obj_id: str) -> None:
        """Assert that an object is visible."""
        obj = await self.get_object(obj_id)
        if not obj:
            from .exceptions import AssertionError_
            raise AssertionError_(f"Object not found: {obj_id}")
        if not obj.visible:
            from .exceptions import AssertionError_
            raise AssertionError_(f"Object not visible: {obj_id}")

    async def assert_text(self, obj_id: str, text: str) -> None:
        """Assert that an object has the expected text."""
        props = await self.get_properties(obj_id)
        actual_text = props.get("text", "")
        if actual_text != text:
            from .exceptions import AssertionError_
            raise AssertionError_(
                f"Text mismatch for {obj_id}: expected='{text}', actual='{actual_text}'"
            )

    async def assert_enabled(self, obj_id: str) -> None:
        """Assert that an object is enabled."""
        obj = await self.get_object(obj_id)
        if not obj:
            from .exceptions import AssertionError_
            raise AssertionError_(f"Object not found: {obj_id}")
        if not obj.enabled:
            from .exceptions import AssertionError_
            raise AssertionError_(f"Object not enabled: {obj_id}")

    # ============ Internal Methods ============

    async def _get(self, path: str, params: dict = None) -> aiohttp.ClientResponse:
        """Send GET request"""
        url = f"{self.base_url}{path}"
        return await self.session.get(url, params=params)

    async def _post(self, path: str, data: dict = None) -> aiohttp.ClientResponse:
        """Send POST request"""
        url = f"{self.base_url}{path}"
        return await self.session.post(url, json=data)

    async def _send_event(self, target: Dict, event: Dict) -> bool:
        """Send an event to the server"""
        try:
            response = await self._post("/events", {
                "target": target,
                "event": event
            })

            if response.status in (400, 404):
                return False

            response.raise_for_status()
            data = await response.json()
            return data.get("success", False)
        except Exception as e:
            logger.warning(f"Failed to send event: {e}")
            return False
