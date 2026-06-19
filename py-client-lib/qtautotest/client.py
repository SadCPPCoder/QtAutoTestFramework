"""
Qt Test Client - Synchronous client for Qt AutoTest Framework
"""

import requests
import base64
import time
import logging
from typing import Dict, Any, List, Optional

from .models import QRect, QObjectInfo, QScreenshot, QObjectTree
from .exceptions import (
    ConnectionError,
    ObjectNotFoundError,
    MultipleObjectsFoundError,
    TimeoutError,
    EventError,
    ServerError,
)

logger = logging.getLogger("qtautotest")


class QtTestClient:
    """
    Synchronous client for Qt AutoTest Framework.

    Usage:
        with QtTestClient("localhost", 8080) as client:
            client.click(objectName="button")
            client.key_type("Hello")
            client.screenshot().save("test.png")
    """

    def __init__(self, host: str = "localhost", port: int = 8080,
                 timeout: int = 30, retry_count: int = 3,
                 retry_interval: float = 1.0, delay: float = 0.1):
        """
        Initialize the client.

        Args:
            host: Server hostname
            port: Server port
            timeout: Request timeout in seconds
            retry_count: Number of retries for failed requests
            retry_interval: Delay between retries in seconds
            delay: Delay between operations in seconds
        """
        self.base_url = f"http://{host}:{port}/api/v1"
        self.timeout = timeout
        self.retry_count = retry_count
        self.retry_interval = retry_interval
        self.delay = delay
        self.session = requests.Session()
        self._default_timeout = 5000

    def connect(self) -> None:
        """Connect to the server and verify health"""
        try:
            response = self.session.get(
                f"{self.base_url}/health",
                timeout=self.timeout
            )
            response.raise_for_status()
            logger.info(f"Connected to {self.base_url}")
        except requests.RequestException as e:
            raise ConnectionError(f"Failed to connect to {self.base_url}: {e}")

    def disconnect(self) -> None:
        """Disconnect from the server"""
        self.session.close()
        logger.info("Disconnected")

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()
        return False

    # ============ Object Query ============

    def find_object(self, **kwargs) -> Optional[QObjectInfo]:
        """
        Find a single object matching the query.

        Args:
            **kwargs: Query parameters (objectName, className, properties, objID, etc.)

        Returns:
            QObjectInfo or None if not found

        Raises:
            MultipleObjectsFoundError: If multiple objects match
        """
        objects = self.find_objects(**kwargs)
        if not objects:
            return None
        if len(objects) > 1:
            raise MultipleObjectsFoundError(query=kwargs, count=len(objects))
        return objects[0]

    def find_objects(self, **kwargs) -> List[QObjectInfo]:
        """
        Find all objects matching the query.

        Args:
            **kwargs: Query parameters

        Returns:
            List of QObjectInfo
        """
        response = self._post("/objects/query", kwargs)

        if response.status_code in (400, 404):
            return []

        response.raise_for_status()
        data = response.json()

        return [QObjectInfo.from_dict(obj) for obj in data.get("objects", [])]

    def get_object(self, obj_id: str) -> Optional[QObjectInfo]:
        """
        Get object by objID.

        Args:
            obj_id: The object ID

        Returns:
            QObjectInfo or None if not found
        """
        try:
            response = self._get(f"/objects/{obj_id}")
            if response.status_code == 404:
                return None
            response.raise_for_status()
            return QObjectInfo.from_dict(response.json())
        except Exception:
            return None

    def get_properties(self, obj_id: str) -> Dict[str, Any]:
        """
        Get all properties of an object.

        Args:
            obj_id: The object ID

        Returns:
            Dictionary of property name to value
        """
        response = self._get(f"/objects/{obj_id}/properties")
        response.raise_for_status()
        return response.json().get("properties", {})

    def get_object_tree(self) -> QObjectTree:
        """
        Get the complete QML object tree.

        Returns:
            QObjectTree root node
        """
        response = self._get("/objects/tree")
        response.raise_for_status()
        return QObjectTree.from_dict(response.json())

    def get_app_info(self) -> Dict[str, Any]:
        """
        Get application information.

        Returns:
            Dictionary with appName, version, qtVersion, etc.
        """
        response = self._get("/app/info")
        response.raise_for_status()
        return response.json()

    # ============ Mouse Events ============

    def click(self, obj_id: str = None, x: int = None, y: int = None,
              button: str = "left", modifiers: List[str] = None) -> bool:
        """
        Click on an object or at coordinates.

        Args:
            obj_id: Object ID to click
            x: X coordinate (relative to object if obj_id set, otherwise window)
            y: Y coordinate
            button: Mouse button (left, right, middle)
            modifiers: List of modifier keys (ctrl, alt, shift)

        Returns:
            True if successful
        """
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

        return self._send_event(target, event)

    def double_click(self, obj_id: str = None, x: int = None,
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

        return self._send_event(target, event)

    def right_click(self, obj_id: str = None, x: int = None,
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

        return self._send_event(target, event)

    # ============ Keyboard Events ============

    def key_type(self, text: str) -> bool:
        """
        Type text character by character.

        Args:
            text: Text to type

        Returns:
            True if successful
        """
        event = {
            "type": "keyboard",
            "action": "type",
            "text": text
        }
        return self._send_event({}, event)

    def key_click(self, key: str, modifiers: List[str] = None) -> bool:
        """
        Press and release a key.

        Args:
            key: Key name (enter, tab, escape, etc.)
            modifiers: List of modifier keys

        Returns:
            True if successful
        """
        event = {
            "type": "keyboard",
            "action": "click",
            "key": key,
            "modifiers": modifiers or []
        }
        return self._send_event({}, event)

    def key_sequence(self, sequence: str) -> bool:
        """
        Execute a key sequence like 'Ctrl+S'.

        Args:
            sequence: Key sequence string

        Returns:
            True if successful
        """
        event = {
            "type": "keyboard",
            "action": "sequence",
            "sequence": sequence
        }
        return self._send_event({}, event)

    # ============ Touch Events ============

    def tap(self, x: int, y: int) -> bool:
        """
        Tap at coordinates.

        Args:
            x: X coordinate
            y: Y coordinate

        Returns:
            True if successful
        """
        event = {
            "type": "touch",
            "action": "tap",
            "x": x,
            "y": y
        }
        return self._send_event({}, event)

    # ============ Wheel Events ============

    def scroll(self, x: int, y: int, delta_x: int = 0,
               delta_y: int = 0) -> bool:
        """
        Scroll at coordinates.

        Args:
            x: X coordinate
            y: Y coordinate
            delta_x: Horizontal scroll amount
            delta_y: Vertical scroll amount

        Returns:
            True if successful
        """
        event = {
            "type": "wheel",
            "action": "scroll",
            "x": x,
            "y": y,
            "deltaX": delta_x,
            "deltaY": delta_y
        }
        return self._send_event({}, event)

    # ============ Screenshot ============

    def screenshot(self, obj_id: str = None) -> Optional[QScreenshot]:
        """
        Take a screenshot.

        Args:
            obj_id: Optional object ID to capture

        Returns:
            QScreenshot or None if failed
        """
        params = {}
        if obj_id:
            params["objID"] = obj_id

        try:
            response = self._get("/screenshot", params=params)

            if response.status_code >= 400:
                return None

            data = response.json()

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

    def wait(self, milliseconds: int) -> None:
        """Wait for specified milliseconds."""
        time.sleep(milliseconds / 1000)

    def wait_for_object(self, timeout: int = None, interval: int = 100,
                        **kwargs) -> Optional[QObjectInfo]:
        """
        Wait for an object to appear.

        Args:
            timeout: Timeout in milliseconds (default: 5000)
            interval: Polling interval in milliseconds
            **kwargs: Query parameters

        Returns:
            QObjectInfo or None if timeout
        """
        if timeout is None:
            timeout = self._default_timeout

        start_time = time.time()
        timeout_sec = timeout / 1000

        while time.time() - start_time < timeout_sec:
            obj = self.find_object(**kwargs)
            if obj:
                return obj
            time.sleep(interval / 1000)

        return None

    def wait_for_object_disappear(self, timeout: int = None,
                                  interval: int = 100,
                                  **kwargs) -> bool:
        """
        Wait for an object to disappear.

        Args:
            timeout: Timeout in milliseconds
            interval: Polling interval in milliseconds
            **kwargs: Query parameters

        Returns:
            True if object disappeared, False if timeout
        """
        if timeout is None:
            timeout = self._default_timeout

        start_time = time.time()
        timeout_sec = timeout / 1000

        while time.time() - start_time < timeout_sec:
            obj = self.find_object(**kwargs)
            if not obj:
                return True
            time.sleep(interval / 1000)

        return False

    # ============ Assertions ============

    def assert_exists(self, **kwargs) -> None:
        """Assert that an object exists."""
        obj = self.find_object(**kwargs)
        if not obj:
            raise AssertionError_(f"Object not found: {kwargs}")

    def assert_not_exists(self, **kwargs) -> None:
        """Assert that an object does not exist."""
        obj = self.find_object(**kwargs)
        if obj:
            raise AssertionError_(f"Object found: {kwargs}")

    def assert_visible(self, obj_id: str) -> None:
        """Assert that an object is visible."""
        obj = self.get_object(obj_id)
        if not obj:
            raise AssertionError_(f"Object not found: {obj_id}")
        if not obj.visible:
            raise AssertionError_(f"Object not visible: {obj_id}")

    def assert_text(self, obj_id: str, text: str) -> None:
        """Assert that an object has the expected text."""
        props = self.get_properties(obj_id)
        actual_text = props.get("text", "")
        if actual_text != text:
            raise AssertionError_(
                f"Text mismatch for {obj_id}: expected='{text}', actual='{actual_text}'"
            )

    def assert_enabled(self, obj_id: str) -> None:
        """Assert that an object is enabled."""
        obj = self.get_object(obj_id)
        if not obj:
            raise AssertionError_(f"Object not found: {obj_id}")
        if not obj.enabled:
            raise AssertionError_(f"Object not enabled: {obj_id}")

    # ============ Configuration ============

    def set_delay(self, seconds: float) -> None:
        """Set delay between operations."""
        self.delay = seconds

    def set_retry(self, count: int, interval: float) -> None:
        """Set retry parameters."""
        self.retry_count = count
        self.retry_interval = interval

    def set_default_timeout(self, milliseconds: int) -> None:
        """Set default timeout for wait operations."""
        self._default_timeout = milliseconds

    # ============ Internal Methods ============

    def _get(self, path: str, params: dict = None) -> requests.Response:
        """Send GET request"""
        url = f"{self.base_url}{path}"
        return self.session.get(url, params=params, timeout=self.timeout)

    def _post(self, path: str, data: dict = None) -> requests.Response:
        """Send POST request"""
        url = f"{self.base_url}{path}"
        return self.session.post(url, json=data, timeout=self.timeout)

    def _send_event(self, target: Dict, event: Dict) -> bool:
        """Send an event to the server"""
        try:
            response = self._post("/events", {
                "target": target,
                "event": event
            })

            if response.status_code in (400, 404):
                return False

            response.raise_for_status()
            return response.json().get("success", False)
        except Exception as e:
            logger.warning(f"Failed to send event: {e}")
            return False
