"""
qtautotest - Python client library for Qt AutoTest Framework

Provides synchronous and asynchronous clients for interacting with
Qt applications that have the QtAutoTestServer integrated.

Usage:
    from qtautotest import QtTestClient

    with QtTestClient("localhost", 8080) as client:
        client.click(objectName="button")
        client.key_type("Hello")
        client.screenshot().save("test.png")
"""

from .client import QtTestClient
from .models import QRect, QObjectInfo, QScreenshot, QObjectTree
from .exceptions import (
    QtTestError,
    ConnectionError,
    ObjectNotFoundError,
    MultipleObjectsFoundError,
    TimeoutError,
    EventError,
    AssertionError_,
    ServerError,
)

# AsyncClient is optional (requires aiohttp)
try:
    from .async_client import AsyncQtTestClient
except ImportError:
    AsyncQtTestClient = None

__version__ = "1.0.0"
__all__ = [
    "QtTestClient",
    "AsyncQtTestClient",
    "QRect",
    "QObjectInfo",
    "QScreenshot",
    "QObjectTree",
    "QtTestError",
    "ConnectionError",
    "ObjectNotFoundError",
    "MultipleObjectsFoundError",
    "TimeoutError",
    "EventError",
    "AssertionError_",
    "ServerError",
]
