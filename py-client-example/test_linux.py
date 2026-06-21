"""
Test connection to Linux QtAutoTestServer
"""

import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'py-client-lib'))
from qtautotest import QtTestClient

SERVER_HOST = "172.29.84.35"
SERVER_PORT = 8080

def main():
    print(f"Connecting to {SERVER_HOST}:{SERVER_PORT}...")
    
    with QtTestClient(SERVER_HOST, SERVER_PORT) as client:
        # Health check
        print("\n=== Health Check ===")
        info = client.get_app_info()
        print(f"  App: {info.get('appName')}")
        print(f"  Qt: {info.get('qtVersion')}")
        print(f"  Objects: {info.get('objectCount')}")
        
        # Object query
        print("\n=== Object Query ===")
        btn = client.get_object("btn_login_001")
        if btn:
            print(f"  Found: {btn.object_name} ({btn.class_name})")
            print(f"  Text: {btn.properties.get('text')}")
            print(f"  Position: ({btn.geometry.x}, {btn.geometry.y})")
        else:
            print("  [WARN] Login button not found")
        
        # Screenshot
        print("\n=== Screenshot ===")
        os.makedirs("screenshots", exist_ok=True)
        screenshot = client.screenshot()
        if screenshot:
            screenshot.save("screenshots/linux_test.png")
            print(f"  Saved: screenshots/linux_test.png ({screenshot.width}x{screenshot.height})")
        else:
            print("  [WARN] Screenshot not available")
        
        print("\n[OK] All tests passed!")

if __name__ == "__main__":
    main()
