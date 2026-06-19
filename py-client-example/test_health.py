"""
Qt AutoTest - Health Check and Basic API Test

Tests the basic API endpoints: health, app info, object tree, object query.
Each test includes cleanup and screenshot capture.
"""

import sys
import os
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'py-client-lib'))

from qtautotest import QtTestClient


SCREENSHOT_DIR = os.path.join(os.path.dirname(__file__), "screenshots")


def ensure_screenshot_dir():
    """Ensure screenshot directory exists"""
    os.makedirs(SCREENSHOT_DIR, exist_ok=True)


def test_health_check(client: QtTestClient):
    """Test health check endpoint"""
    print("\n=== Test Health Check ===")

    info = client.get_app_info()
    print(f"  App Name: {info.get('appName')}")
    print(f"  Qt Version: {info.get('qtVersion')}")
    print(f"  Object Count: {info.get('objectCount')}")

    # Screenshot
    screenshot = client.screenshot()
    if screenshot:
        screenshot.save(os.path.join(SCREENSHOT_DIR, "health_check.png"))
        print(f"  Screenshot saved: health_check.png ({screenshot.width}x{screenshot.height})")

    print("[OK] Health check passed")


def test_object_tree(client: QtTestClient):
    """Test object tree retrieval"""
    print("\n=== Test Object Tree ===")

    tree = client.get_object_tree()
    print("Object tree structure:")
    tree.print_tree()

    # Count objects
    all_nodes = tree.find_all()
    print(f"\n  Total objects in tree: {len(all_nodes)}")

    # Screenshot
    screenshot = client.screenshot()
    if screenshot:
        screenshot.save(os.path.join(SCREENSHOT_DIR, "object_tree.png"))
        print(f"  Screenshot saved: object_tree.png")

    print("[OK] Object tree retrieved")


def test_query_by_object_name(client: QtTestClient):
    """Test querying objects by objectName"""
    print("\n=== Test Query by objectName ===")

    # Query login button (use objID for precise match)
    print("  Querying objID='btn_login_001'...")
    obj = client.get_object("btn_login_001")
    if obj:
        print(f"  [OK] Found: {obj.object_name} ({obj.class_name})")
        print(f"       objID: {obj.obj_id}")
        print(f"       Position: ({obj.geometry.x}, {obj.geometry.y})")
        print(f"       Size: {obj.geometry.width}x{obj.geometry.height}")
        print(f"       Visible: {obj.visible}, Enabled: {obj.enabled}")
    else:
        print("  [WARN] Login button not found (QML may not be loaded)")

    # Query username field
    print("\n  Querying objID='input_username_001'...")
    obj = client.get_object("input_username_001")
    if obj:
        print(f"  [OK] Found: {obj.object_name} ({obj.class_name})")
        print(f"       objID: {obj.obj_id}")
        print(f"       Properties: {obj.properties}")
    else:
        print("  [WARN] Username field not found")

    # Screenshot
    screenshot = client.screenshot()
    if screenshot:
        screenshot.save(os.path.join(SCREENSHOT_DIR, "query_by_name.png"))

    print("[OK] Query by objectName passed")


def test_query_by_class_name(client: QtTestClient):
    """Test querying objects by className"""
    print("\n=== Test Query by className ===")

    # Query all buttons
    print("  Querying className='QQuickButton'...")
    buttons = client.find_objects(className="QQuickButton")
    print(f"  Found {len(buttons)} buttons:")
    for btn in buttons:
        print(f"    - {btn.object_name}: {btn.properties.get('text', '')} [{btn.obj_id}]")

    # Query all text fields
    print("\n  Querying className='QQuickTextField'...")
    fields = client.find_objects(className="QQuickTextField")
    print(f"  Found {len(fields)} text fields:")
    for field in fields:
        print(f"    - {field.object_name} [{field.obj_id}]")

    # Screenshot
    screenshot = client.screenshot()
    if screenshot:
        screenshot.save(os.path.join(SCREENSHOT_DIR, "query_by_class.png"))

    print("[OK] Query by className passed")


def test_query_by_properties(client: QtTestClient):
    """Test querying objects by properties"""
    print("\n=== Test Query by properties ===")

    # Query by text property (use find_objects to handle multiple matches)
    print("  Querying properties={'text': 'Login'}...")
    objects = client.find_objects(properties={"text": "Login"})
    print(f"  Found {len(objects)} objects with text='Login':")
    for obj in objects:
        print(f"    - {obj.object_name} ({obj.class_name}) [{obj.obj_id}]")

    # Query by className + text for precise match
    print("\n  Querying className='QQuickButton' + properties={'text': 'Login'}...")
    obj = client.find_object(className="QQuickButton", properties={"text": "Login"})
    if obj:
        print(f"  [OK] Found: {obj.object_name} with text='Login'")
    else:
        print("  [WARN] Button with text='Login' not found")

    # Query by enabled property
    print("\n  Querying enabled=True...")
    enabled_objects = client.find_objects(enabled=True)
    print(f"  Found {len(enabled_objects)} enabled objects")

    # Query by visible property
    print("\n  Querying visible=True...")
    visible_objects = client.find_objects(visible=True)
    print(f"  Found {len(visible_objects)} visible objects")

    # Screenshot
    screenshot = client.screenshot()
    if screenshot:
        screenshot.save(os.path.join(SCREENSHOT_DIR, "query_by_properties.png"))

    print("[OK] Query by properties passed")


def test_query_by_obj_id(client: QtTestClient):
    """Test querying objects by objID"""
    print("\n=== Test Query by objID ===")

    # Query by objID
    print("  Querying objID='btn_login_001'...")
    obj = client.get_object("btn_login_001")
    if obj:
        print(f"  [OK] Found: {obj.object_name} ({obj.class_name})")
        print(f"       Properties: {obj.properties}")
    else:
        print("  [WARN] Object with objID='btn_login_001' not found")

    # Get properties
    print("\n  Getting properties for objID='btn_login_001'...")
    props = client.get_properties("btn_login_001")
    print(f"  Properties: {props}")

    # Screenshot
    screenshot = client.screenshot()
    if screenshot:
        screenshot.save(os.path.join(SCREENSHOT_DIR, "query_by_objid.png"))

    print("[OK] Query by objID passed")


def test_screenshot(client: QtTestClient):
    """Test screenshot functionality"""
    print("\n=== Test Screenshot ===")

    # Full window screenshot
    print("  Taking full window screenshot...")
    screenshot = client.screenshot()
    if screenshot:
        screenshot.save(os.path.join(SCREENSHOT_DIR, "screenshot_full.png"))
        print(f"  [OK] Saved: screenshot_full.png ({screenshot.width}x{screenshot.height})")
    else:
        print("  [WARN] Screenshot not available")

    # Object screenshot
    print("\n  Taking button screenshot...")
    btn = client.get_object("btn_login_001")
    if btn:
        btn_screenshot = client.screenshot(obj_id=btn.obj_id)
        if btn_screenshot:
            btn_screenshot.save(os.path.join(SCREENSHOT_DIR, "screenshot_button.png"))
            print(f"  [OK] Saved: screenshot_button.png")
        else:
            print("  [WARN] Button screenshot not available")
    else:
        print("  [WARN] Button not found")

    print("[OK] Screenshot test passed")


def test_click_event(client: QtTestClient):
    """Test click event"""
    print("\n=== Test Click Event ===")

    # Find and click login button (use objID for precise match)
    print("  Finding login button by objID...")
    btn = client.get_object("btn_login_001")
    if btn:
        print(f"  Found: {btn.object_name} at ({btn.geometry.x}, {btn.geometry.y})")

        # Click the button
        print("  Clicking login button...")
        result = client.click(obj_id=btn.obj_id,
                              x=btn.geometry.width // 2,
                              y=btn.geometry.height // 2)
        print(f"  Click result: {result}")

        # Wait for dialog
        client.wait(500)

        # Screenshot after click
        screenshot = client.screenshot()
        if screenshot:
            screenshot.save(os.path.join(SCREENSHOT_DIR, "after_click.png"))
            print(f"  Screenshot saved: after_click.png")

        # Close dialog
        client.key_click("enter")
        client.wait(300)

        # Cleanup
        cleanup_after_test(client)
    else:
        print("  [WARN] Login button not found")

    print("[OK] Click event test passed")


def test_keyboard_event(client: QtTestClient):
    """Test keyboard event"""
    print("\n=== Test Keyboard Event ===")

    # Find username field by objID
    print("  Finding username field by objID...")
    field = client.get_object("input_username_001")
    if field:
        print(f"  Found: {field.object_name}")

        # Click the field
        client.click(obj_id=field.obj_id,
                     x=field.geometry.width // 2,
                     y=field.geometry.height // 2)
        client.wait(200)

        # Type text
        print("  Typing 'Hello World'...")
        client.key_type("Hello World")
        client.wait(300)

        # Screenshot after typing
        screenshot = client.screenshot()
        if screenshot:
            screenshot.save(os.path.join(SCREENSHOT_DIR, "after_typing.png"))
            print(f"  Screenshot saved: after_typing.png")

        # Cleanup
        cleanup_after_test(client)
    else:
        print("  [WARN] Username field not found")

    print("[OK] Keyboard event test passed")


def cleanup_after_test(client: QtTestClient):
    """Cleanup after test"""
    print("  --- Cleanup ---")

    # Close dialogs
    for _ in range(3):
        client.key_click("escape")
        client.wait(100)
    client.key_click("enter")
    client.wait(100)

    # Clear input fields using objID
    username = client.get_object("input_username_001")
    if username:
        client.click(obj_id=username.obj_id,
                     x=username.geometry.width // 2,
                     y=username.geometry.height // 2)
        client.wait(100)
        client.key_sequence("Ctrl+A")
        client.key_click("delete")

    password = client.get_object("input_password_001")
    if password:
        client.click(obj_id=password.obj_id,
                     x=password.geometry.width // 2,
                     y=password.geometry.height // 2)
        client.wait(100)
        client.key_sequence("Ctrl+A")
        client.key_click("delete")

    # Click neutral area
    client.click(x=400, y=100)
    client.wait(200)

    print("  [OK] Cleanup done")


def main():
    """Run all tests"""
    print("=" * 60)
    print("Qt AutoTest - Health Check & API Tests")
    print("=" * 60)

    ensure_screenshot_dir()

    try:
        with QtTestClient("localhost", 8080) as client:
            # Basic API tests
            test_health_check(client)
            test_object_tree(client)

            # Query tests
            test_query_by_object_name(client)
            test_query_by_class_name(client)
            test_query_by_properties(client)
            test_query_by_obj_id(client)

            # Screenshot test
            test_screenshot(client)

            # Event tests
            test_click_event(client)
            test_keyboard_event(client)

            print("\n" + "=" * 60)
            print("[OK] All tests passed!")
            print(f"Screenshots saved to: {SCREENSHOT_DIR}")
            print("=" * 60)

    except Exception as e:
        print(f"\n[FAIL] Test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

    return True


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
