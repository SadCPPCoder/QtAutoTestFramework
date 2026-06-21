"""
Test with delays to observe UI changes on Linux
"""

import sys
import os
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'py-client-lib'))
from qtautotest import QtTestClient

SERVER_HOST = "172.29.84.35"
SERVER_PORT = 8080

def wait(seconds, message=""):
    if message:
        print(f"  {message}... ", end="", flush=True)
    for i in range(seconds, 0, -1):
        print(f"{i} ", end="", flush=True)
        time.sleep(1)
    print("Done")

def main():
    print(f"Connecting to {SERVER_HOST}:{SERVER_PORT}...")
    
    with QtTestClient(SERVER_HOST, SERVER_PORT) as client:
        
        # Step 1: Health check
        print("\n=== Step 1: Health Check ===")
        info = client.get_app_info()
        print(f"  App: {info.get('appName')}")
        wait(2, "Look at the UI")
        
        # Step 2: Find username field and click
        print("\n=== Step 2: Click Username Field ===")
        field = client.get_object("input_username_001")
        if field:
            print(f"  Found: {field.object_name}")
            client.click(obj_id=field.obj_id,
                         x=field.geometry.width // 2,
                         y=field.geometry.height // 2)
            print("  [OK] Clicked username field")
        wait(2, "Watch the focus")
        
        # Step 3: Type username slowly
        print("\n=== Step 3: Type Username 'admin' ===")
        for i, char in enumerate("admin", 1):
            client.key_type(char)
            print(f"  Typed '{char}' ({i}/5)")
            time.sleep(0.5)
        wait(2, "Watch the text appear")
        
        # Step 4: Click password field
        print("\n=== Step 4: Click Password Field ===")
        field = client.get_object("input_password_001")
        if field:
            client.click(obj_id=field.obj_id,
                         x=field.geometry.width // 2,
                         y=field.geometry.height // 2)
            print("  [OK] Clicked password field")
        wait(2, "Watch the focus change")
        
        # Step 5: Type password slowly
        print("\n=== Step 5: Type Password 'password' ===")
        for i, char in enumerate("password", 1):
            client.key_type(char)
            print(f"  Typed '{char}' ({i}/8)")
            time.sleep(0.5)
        wait(2, "Watch the text appear")
        
        # Step 6: Click login button
        print("\n=== Step 6: Click Login Button ===")
        btn = client.get_object("btn_login_001")
        if btn:
            client.click(obj_id=btn.obj_id,
                         x=btn.geometry.width // 2,
                         y=btn.geometry.height // 2)
            print("  [OK] Clicked login button")
        wait(3, "Watch for dialog")
        
        # Step 7: Close dialog
        print("\n=== Step 7: Close Dialog ===")
        client.key_click("enter")
        print("  [OK] Pressed Enter")
        wait(1, "Watch dialog close")
        
        # Cleanup
        print("\n=== Cleanup ===")
        for _ in range(3):
            client.key_click("escape")
            time.sleep(0.2)
        client.key_click("enter")
        time.sleep(0.2)
        
        field = client.get_object("input_username_001")
        if field:
            client.click(obj_id=field.obj_id,
                         x=field.geometry.width // 2,
                         y=field.geometry.height // 2)
            client.key_sequence("Ctrl+A")
            client.key_click("delete")
        
        field = client.get_object("input_password_001")
        if field:
            client.click(obj_id=field.obj_id,
                         x=field.geometry.width // 2,
                         y=field.geometry.height // 2)
            client.key_sequence("Ctrl+A")
            client.key_click("delete")
        
        print("  [OK] Cleanup done")
        
        print("\n" + "=" * 60)
        print("[OK] TEST COMPLETED!")
        print("=" * 60)

if __name__ == "__main__":
    main()
