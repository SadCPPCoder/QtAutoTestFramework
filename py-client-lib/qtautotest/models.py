"""
Data models for qtautotest
"""

import base64
from dataclasses import dataclass, field
from typing import Dict, Any, List, Optional


@dataclass
class QRect:
    """Rectangle representing position and size"""
    x: int = 0
    y: int = 0
    width: int = 0
    height: int = 0

    @property
    def center(self) -> tuple:
        """Center point coordinates"""
        return (self.x + self.width // 2, self.y + self.height // 2)

    @property
    def right(self) -> int:
        """Right edge"""
        return self.x + self.width

    @property
    def bottom(self) -> int:
        """Bottom edge"""
        return self.y + self.height

    @classmethod
    def from_dict(cls, data: dict) -> 'QRect':
        """Create from dictionary"""
        return cls(
            x=data.get("x", 0),
            y=data.get("y", 0),
            width=data.get("width", 0),
            height=data.get("height", 0)
        )


@dataclass
class QObjectInfo:
    """QML object information"""
    obj_id: str = ""
    object_name: str = ""
    class_name: str = ""
    properties: Dict[str, Any] = field(default_factory=dict)
    geometry: QRect = field(default_factory=QRect)
    visible: bool = True
    enabled: bool = True
    path: str = ""

    @classmethod
    def from_dict(cls, data: dict) -> 'QObjectInfo':
        """Create from dictionary"""
        geometry = QRect.from_dict(data.get("geometry", {}))
        return cls(
            obj_id=data.get("objID", ""),
            object_name=data.get("objectName", ""),
            class_name=data.get("className", ""),
            properties=data.get("properties", {}),
            geometry=geometry,
            visible=data.get("visible", True),
            enabled=data.get("enabled", True),
            path=data.get("path", "")
        )

    @property
    def center(self) -> tuple:
        """Center point of the object"""
        return self.geometry.center


class QScreenshot:
    """Screenshot image wrapper"""

    def __init__(self, image_data: bytes, width: int, height: int):
        self.image_data = image_data
        self.width = width
        self.height = height

    @classmethod
    def from_base64(cls, data: str, width: int, height: int) -> 'QScreenshot':
        """Create from Base64 encoded string"""
        image_data = base64.b64decode(data)
        return cls(image_data, width, height)

    def save(self, path: str) -> None:
        """Save screenshot to file"""
        with open(path, 'wb') as f:
            f.write(self.image_data)

    def show(self) -> None:
        """Show image (for debugging)"""
        import io
        from PIL import Image
        img = Image.open(io.BytesIO(self.image_data))
        img.show()

    def to_pil(self):
        """Convert to PIL Image"""
        import io
        from PIL import Image
        return Image.open(io.BytesIO(self.image_data))

    def to_base64(self) -> str:
        """Convert to Base64 string"""
        return base64.b64encode(self.image_data).decode('utf-8')


class QObjectTree:
    """QML object tree node"""

    def __init__(self, obj_id: str = "", object_name: str = "", class_name: str = "",
                 properties: Dict[str, Any] = None, children: List['QObjectTree'] = None):
        self.obj_id = obj_id
        self.object_name = object_name
        self.class_name = class_name
        self.properties = properties or {}
        self.children = children or []

    @classmethod
    def from_dict(cls, data: dict) -> 'QObjectTree':
        """Create from dictionary"""
        children = [cls.from_dict(c) for c in data.get("children", [])]
        return cls(
            obj_id=data.get("objID", ""),
            object_name=data.get("objectName", ""),
            class_name=data.get("className", ""),
            properties=data.get("properties", {}),
            children=children
        )

    def find(self, **kwargs) -> Optional['QObjectTree']:
        """Find first matching node"""
        if self._matches(kwargs):
            return self

        for child in self.children:
            result = child.find(**kwargs)
            if result:
                return result

        return None

    def find_all(self, **kwargs) -> List['QObjectTree']:
        """Find all matching nodes"""
        results = []

        if self._matches(kwargs):
            results.append(self)

        for child in self.children:
            results.extend(child.find_all(**kwargs))

        return results

    def _matches(self, kwargs: dict) -> bool:
        """Check if this node matches the query"""
        if "objectName" in kwargs and self.object_name != kwargs["objectName"]:
            return False
        if "className" in kwargs and self.class_name != kwargs["className"]:
            return False
        if "objID" in kwargs and self.obj_id != kwargs["objID"]:
            return False
        return True

    def print_tree(self, indent: int = 0) -> None:
        """Print the object tree"""
        prefix = "  " * indent
        id_str = f" [{self.obj_id}]" if self.obj_id else ""
        print(f"{prefix}{self.class_name}: {self.object_name}{id_str}")

        for child in self.children:
            child.print_tree(indent + 1)

    def to_dict(self) -> dict:
        """Convert to dictionary"""
        return {
            "objID": self.obj_id,
            "objectName": self.object_name,
            "className": self.class_name,
            "properties": self.properties,
            "children": [c.to_dict() for c in self.children]
        }
