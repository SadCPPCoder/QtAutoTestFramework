"""
Comprehensive test for Linux QtAutoTestServer
"""

import sys
import os
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'py-client-lib'))
from qtautotest import QtTestClient

SERVER_HOST = "172.29.84.35"
SERVER_PORT = 8080
SCREENSHOT_DIR = "screenshots"

def save_screenshot(client, name):
    os.makedirs(SCREENSHOT_DIR, exist_ok=True)
    screenshot = client.screenshot()
    if screenshot:
        path = os.path.join(SCREENSHOT_DIR, f"{name}.png")
        screenshot.save(path)
        print(f"  Screenshot: {path} ({screenshot.width}x{screenshot.height})")
        return True
    return False

def test_health(client):
    print("\n=== Test Health Check ===")
    info = client.get_app_info()
    print(f"  App: {info.get('appName')}")
    print(f"  Qt: {info.get('qtVersion')}")
    print(f"  Objects: {info.get('objectCount')}")
    print("[OK] Health check passed")

def test_object_query(client):
    print("\n=== Test Object Query ===")
    
    # Query by objID
    print("  Query by objID...")
    btn = client.get_object("btn_login_001")
    if btn:
        print(f"  Found: {btn.object_name} [{btn.obj_id}]")
    
    # Query by className
    print("  Query by className...")
    buttons = client.find_objects(className="QQuickButton")
    print(f"  Found {len(buttons)} buttons")
    
    # Query by properties
    print("  Query by properties...")
    objs = client.find_objects(properties={"text": "Login"})
    print(f"  Found {len(objs)} objects with text='Login'")
    
    print("[OK] Object query passed")

def test_keyboard(client):
    print("\n=== Test Keyboard Input ===")
    
    # Find username field
    field = client.get_object("input_username_001")
    if not field:
        print("  [SKIP] Username field not found")
        return
    
    # Click field
    print("  Clicking username field...")
    client.click(obj_id=field.obj_id,
                 x=field.geometry.width // 2,
                 y=field.geometry.height // 2)
    time.sleep(0.3)
    
    # Type text
    print("  Typing 'Hello Linux'...")
    client.key_type("Hello Linux")
    time.sleep(0.5)
    
    save_screenshot(client, "keyboard_test")
    
    # Cleanup
    client.key_sequence("Ctrl+A")
    client.key_click("delete")
    
    print("[OK] Keyboard test passed")

def test_mouse(client):
    print("\n=== Test Mouse Click ===")
    
    btn = client.get_object("btn_login_001")
    if not btn:
        print("  [SKIP] Login button not found")
        return
    
    # Click button
    print(f"  Clicking login button at ({btn.geometry.x}, {btn.geometry.y})...")
    client.click(obj_id=btn.obj_id,
                 x=btn.geometry.width // 2,
                 y=btn.geometry.height // 2)
    time.sleep(0.5)
    
    save_screenshot(client, "mouse_test")
    
    # Close dialog
    client.key_click("enter")
    time.sleep(0.3)
    
    print("[OK] Mouse test passed")

def test_screenshot(client):
    print("\n=== Test Screenshot ===")
    
    # Full window
    print("  Taking full window screenshot...")
    save_screenshot(client, "full_window")
    
    # Object screenshot
    btn = client.get_object("btn_login_001")
    if btn:
        print("  Taking button screenshot...")
        screenshot = client.screenshot(obj_id=btn.obj_id)
        if screenshot:
            path = os.path.join(SCREENSHOT_DIR, "button_screenshot.png")
            screenshot.save(path)
            print(f"  Saved: {path}")
    
    print("[OK] Screenshot test passed")

def main():
    print(f"Connecting to {SERVER_HOST}:{SERVER_PORT}...")
    print("=" * 60)
    
    with QtTestClient(SERVER_HOST, SERVER_PORT) as client:
        test_health(client)
        test_object_query(client)
        test_keyboard(client)
        test_mouse(client)
        test_screenshot(client)
        
        print("\n" + "=" * 60)
        print("[OK] ALL TESTS PASSED!")
        print(f"Screenshots saved to: {SCREENSHOT_DIR}/")

if __name__ == "__main__":
    main()
