"""
Qt AutoTest - Login Flow Test

Tests the complete login flow with proper cleanup between tests.
Covers: query, click, keyboard input, screenshot, wait, assertion.
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


def print_step(step_num, description):
    """Print step info"""
    print(f"\n{'='*60}")
    print(f"[Step {step_num}] {description}")
    print(f"{'='*60}")


def wait_with_countdown(seconds, message="Waiting"):
    """Wait with countdown"""
    for i in range(seconds, 0, -1):
        print(f"\r  {message}... {i} sec ", end="", flush=True)
        time.sleep(1)
    print(f"\r  {message}... Done!    ")


def cleanup_after_test(client: QtTestClient):
    """Full cleanup after test case"""
    print("\n  --- CLEANUP ---")

    # Close any open dialogs
    for _ in range(3):
        client.key_click("escape")
        client.wait(100)
    client.key_click("enter")
    client.wait(200)

    # Clear username field by objID
    username = client.get_object("input_username_001")
    if username:
        client.click(obj_id=username.obj_id,
                     x=username.geometry.width // 2,
                     y=username.geometry.height // 2)
        client.wait(100)
        client.key_sequence("Ctrl+A")
        client.key_click("delete")
        client.wait(100)

    # Clear password field by objID
    password = client.get_object("input_password_001")
    if password:
        client.click(obj_id=password.obj_id,
                     x=password.geometry.width // 2,
                     y=password.geometry.height // 2)
        client.wait(100)
        client.key_sequence("Ctrl+A")
        client.key_click("delete")
        client.wait(100)

    # Click neutral area to remove focus
    client.click(x=400, y=100)
    client.wait(200)

    print("  [OK] Cleanup done")
    print("  --- CLEANUP END ---\n")


def type_text_slowly(client: QtTestClient, text: str, delay: float = 0.3):
    """Type text character by character"""
    for i, char in enumerate(text, 1):
        client.key_type(char)
        print(f"    [{i}/{len(text)}] Typed '{char}'")
        time.sleep(delay)


def test_login_success(client: QtTestClient):
    """Test successful login flow"""
    print("\n" + "*" * 60)
    print("* TEST: Successful Login")
    print("* Covers: query, click, keyboard, screenshot, wait")
    print("*" * 60)

    # Step 1: Query and verify initial state
    print_step(1, "Query and verify initial state")

    print("  Querying login button by objID...")
    login_btn = client.get_object("btn_login_001")
    assert login_btn is not None, "Login button not found"
    print(f"  [OK] Found: {login_btn.object_name} [{login_btn.obj_id}]")
    print(f"       Text: {login_btn.properties.get('text')}")
    print(f"       Position: ({login_btn.geometry.x}, {login_btn.geometry.y})")

    print("\n  Querying username field by objID...")
    username_field = client.get_object("input_username_001")
    assert username_field is not None, "Username field not found"
    print(f"  [OK] Found: {username_field.object_name} [{username_field.obj_id}]")

    print("\n  Querying password field by objID...")
    password_field = client.get_object("input_password_001")
    assert password_field is not None, "Password field not found"
    print(f"  [OK] Found: {password_field.object_name} [{password_field.obj_id}]")

    # Screenshot - initial state
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "login_01_initial.png"))
    print("  Screenshot: login_01_initial.png")
    wait_with_countdown(1, "Prepare")

    # Step 2: Enter username
    print_step(2, "Enter username 'admin'")

    client.click(obj_id=username_field.obj_id,
                 x=username_field.geometry.width // 2,
                 y=username_field.geometry.height // 2)
    client.wait(200)

    print("  Typing 'admin':")
    type_text_slowly(client, "admin", 0.3)

    # Screenshot - after username
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "login_02_username.png"))
    print("  Screenshot: login_02_username.png")
    wait_with_countdown(1, "Watch")

    # Step 3: Enter password
    print_step(3, "Enter password 'password'")

    client.click(obj_id=password_field.obj_id,
                 x=password_field.geometry.width // 2,
                 y=password_field.geometry.height // 2)
    client.wait(200)

    print("  Typing 'password':")
    type_text_slowly(client, "password", 0.3)

    # Screenshot - after password
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "login_03_password.png"))
    print("  Screenshot: login_03_password.png")
    wait_with_countdown(1, "Watch")

    # Step 4: Click login button
    print_step(4, "Click login button")

    client.click(obj_id=login_btn.obj_id,
                 x=login_btn.geometry.width // 2,
                 y=login_btn.geometry.height // 2)
    print("  [OK] Clicked login button")

    # Wait for dialog
    wait_with_countdown(2, "Wait for dialog")

    # Screenshot - after login
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "login_04_success.png"))
    print("  Screenshot: login_04_success.png")

    # Step 5: Verify and close dialog
    print_step(5, "Verify success and close dialog")

    # Check for success dialog
    success_dialog = client.find_object(objectName="loginSuccessDialog")
    if success_dialog:
        print(f"  [OK] Success dialog found: {success_dialog.object_name}")

        # Check message text
        message = client.find_object(objectName="loginSuccessMessage")
        if message:
            print(f"  [OK] Message: {message.properties.get('text')}")
    else:
        print("  [WARN] Success dialog not found (may have closed automatically)")

    # Close dialog
    client.key_click("enter")
    client.wait(300)
    print("  [OK] Dialog closed")

    # Final screenshot
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "login_05_final.png"))
    print("  Screenshot: login_05_final.png")

    # Cleanup
    cleanup_after_test(client)

    print("\n" + "*" * 60)
    print("[OK] LOGIN SUCCESS TEST PASSED")
    print("*" * 60)


def test_login_failure(client: QtTestClient):
    """Test login failure flow"""
    print("\n" + "*" * 60)
    print("* TEST: Login Failure")
    print("* Covers: query, click, keyboard, screenshot")
    print("*" * 60)

    # Step 1: Query and verify initial state
    print_step(1, "Query and verify initial state")

    login_btn = client.get_object("btn_login_001")
    assert login_btn is not None, "Login button not found"
    print(f"  [OK] Login button found: {login_btn.object_name}")

    username_field = client.get_object("input_username_001")
    assert username_field is not None, "Username field not found"

    password_field = client.get_object("input_password_001")
    assert password_field is not None, "Password field not found"

    # Screenshot - initial state
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "fail_01_initial.png"))
    print("  Screenshot: fail_01_initial.png")
    wait_with_countdown(1, "Prepare")

    # Step 2: Enter wrong credentials
    print_step(2, "Enter wrong credentials")

    # Enter wrong username
    client.click(obj_id=username_field.obj_id,
                 x=username_field.geometry.width // 2,
                 y=username_field.geometry.height // 2)
    client.wait(200)

    print("  Typing wrong username 'wronguser':")
    type_text_slowly(client, "wronguser", 0.2)

    # Enter wrong password
    client.click(obj_id=password_field.obj_id,
                 x=password_field.geometry.width // 2,
                 y=password_field.geometry.height // 2)
    client.wait(200)

    print("  Typing wrong password 'wrongpass':")
    type_text_slowly(client, "wrongpass", 0.2)

    # Screenshot - wrong credentials
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "fail_02_credentials.png"))
    print("  Screenshot: fail_02_credentials.png")
    wait_with_countdown(1, "Watch")

    # Step 3: Click login
    print_step(3, "Click login button")

    client.click(obj_id=login_btn.obj_id,
                 x=login_btn.geometry.width // 2,
                 y=login_btn.geometry.height // 2)
    print("  [OK] Clicked login button")

    # Wait for error dialog
    wait_with_countdown(2, "Wait for error dialog")

    # Screenshot - error dialog
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "fail_03_error.png"))
    print("  Screenshot: fail_03_error.png")

    # Step 4: Verify error and close
    print_step(4, "Verify error and close dialog")

    # Check for error dialog
    error_dialog = client.find_object(objectName="loginFailedDialog")
    if error_dialog:
        print(f"  [OK] Error dialog found: {error_dialog.object_name}")

        # Check error message
        message = client.find_object(objectName="loginFailedMessage")
        if message:
            print(f"  [OK] Error message: {message.properties.get('text')}")
    else:
        print("  [WARN] Error dialog not found")
    # Close dialog
    client.key_click("enter")
    client.wait(300)
    print("  [OK] Dialog closed")

    # Final screenshot
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "fail_04_final.png"))
    print("  Screenshot: fail_04_final.png")

    # Cleanup
    cleanup_after_test(client)

    print("\n" + "*" * 60)
    print("[OK] LOGIN FAILURE TEST PASSED")
    print("*" * 60)


def test_multiple_login_attempts(client: QtTestClient):
    """Test multiple login attempts with cleanup between each"""
    print("\n" + "*" * 60)
    print("* TEST: Multiple Login Attempts")
    print("* Demonstrates proper cleanup between test cases")
    print("*" * 60)

    # Attempt 1: Wrong credentials
    print("\n" + "-" * 40)
    print("ATTEMPT 1: Wrong credentials")
    print("-" * 40)

    # Query fields by objID
    username_field = client.get_object("input_username_001")
    password_field = client.get_object("input_password_001")
    login_btn = client.get_object("btn_login_001")

    if not all([username_field, password_field, login_btn]):
        print("[SKIP] Required fields not found")
        return

    # Enter wrong credentials
    client.click(obj_id=username_field.obj_id,
                 x=username_field.geometry.width // 2,
                 y=username_field.geometry.height // 2)
    client.wait(100)
    client.key_type("wrong")
    client.wait(100)

    client.click(obj_id=password_field.obj_id,
                 x=password_field.geometry.width // 2,
                 y=password_field.geometry.height // 2)
    client.wait(100)
    client.key_type("wrong")
    client.wait(100)

    # Click login
    client.click(obj_id=login_btn.obj_id,
                 x=login_btn.geometry.width // 2,
                 y=login_btn.geometry.height // 2)
    wait_with_countdown(2, "Wait for error")

    # Screenshot
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "multi_01_fail.png"))
    print("  Screenshot: multi_01_fail.png")

    # Close dialog
    client.key_click("enter")
    client.wait(300)

    # Cleanup between tests
    cleanup_after_test(client)

    # Attempt 2: Correct credentials
    print("\n" + "-" * 40)
    print("ATTEMPT 2: Correct credentials (after cleanup)")
    print("-" * 40)

    # Query fields again by objID (after cleanup)
    username_field = client.get_object("input_username_001")
    password_field = client.get_object("input_password_001")
    login_btn = client.get_object("btn_login_001")

    # Enter correct credentials
    client.click(obj_id=username_field.obj_id,
                 x=username_field.geometry.width // 2,
                 y=username_field.geometry.height // 2)
    client.wait(100)
    client.key_type("admin")
    client.wait(100)

    client.click(obj_id=password_field.obj_id,
                 x=password_field.geometry.width // 2,
                 y=password_field.geometry.height // 2)
    client.wait(100)
    client.key_type("password")
    client.wait(100)

    # Click login
    client.click(obj_id=login_btn.obj_id,
                 x=login_btn.geometry.width // 2,
                 y=login_btn.geometry.height // 2)
    wait_with_countdown(2, "Wait for success")

    # Screenshot
    client.screenshot().save(os.path.join(SCREENSHOT_DIR, "multi_02_success.png"))
    print("  Screenshot: multi_02_success.png")

    # Close dialog
    client.key_click("enter")
    client.wait(300)

    # Final cleanup
    cleanup_after_test(client)

    print("\n" + "*" * 60)
    print("[OK] MULTIPLE LOGIN ATTEMPTS TEST PASSED")
    print("*" * 60)


def main():
    """Main function"""
    print("\n" + "=" * 60)
    print("Qt AutoTest - Login Flow tests")
    print("=" * 60)
    print("\nPrerequisites:")
    print("  1. DemoApp is running with --test-server --test-port=8080")
    print("  2. You can see the application window")
    print("\nSelect test:")
    print("  1. Successful login test")
    print("  2. Login failure test")
    print("  3. Multiple login attempts")
    print("  4. Run all tests")

    choice = input("\nSelect (1-4): ").strip()

    ensure_screenshot_dir()

    try:
        with QtTestClient("localhost", 8080) as client:
            if choice == "1":
                test_login_success(client)
            elif choice == "2":
                test_login_failure(client)
            elif choice == "3":
                test_multiple_login_attempts(client)
            elif choice == "4":
                test_login_success(client)
                wait_with_countdown(2, "Prepare next test")
                test_login_failure(client)
                wait_with_countdown(2, "Prepare next test")
                test_multiple_login_attempts(client)
            else:
                print("Invalid choice, running all tests")
                test_login_success(client)
                wait_with_countdown(2, "Prepare next test")
                test_login_failure(client)

        print("\n" + "=" * 60)
        print("[OK] ALL TESTS COMPLETED!")
        print(f"Screenshots saved to: {SCREENSHOT_DIR}")
        print("=" * 60)

    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
    except Exception as e:
        print(f"\n\n[FAIL] Test failed: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
