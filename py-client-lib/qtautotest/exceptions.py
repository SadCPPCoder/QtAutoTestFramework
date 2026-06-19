"""
Exception classes for qtautotest
"""


class QtTestError(Exception):
    """Base exception for all qtautotest errors"""
    pass


class ConnectionError(QtTestError):
    """Failed to connect to the test server"""
    pass


class ObjectNotFoundError(QtTestError):
    """Requested QML object was not found"""

    def __init__(self, query: dict = None, message: str = None):
        self.query = query or {}
        super().__init__(message or f"Object not found: {self.query}")


class MultipleObjectsFoundError(QtTestError):
    """Multiple objects found when only one was expected"""

    def __init__(self, query: dict = None, count: int = 0):
        self.query = query or {}
        self.count = count
        super().__init__(f"Found {count} objects, expected 1: {self.query}")


class TimeoutError(QtTestError):
    """Operation timed out"""

    def __init__(self, timeout: int = None, message: str = None):
        self.timeout = timeout
        super().__init__(message or f"Operation timed out after {timeout}ms")


class EventError(QtTestError):
    """Failed to send or process an event"""
    pass


class AssertionError_(QtTestError):
    """Assertion failed"""
    pass


class ServerError(QtTestError):
    """Server returned an error"""

    def __init__(self, code: str = None, message: str = None):
        self.code = code
        super().__init__(message or f"Server error: {code}")
