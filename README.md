import argparse
import time
import sys
from utils.app_utils import install_app, uninstall_app, verify_app_status

VERIFICATION_DELAY_SECONDS = 30  # 설치/삭제 후 상태가 반영될 때까지 대기하는 시간

def run_sota_test(app_id: str, app_version: str, app_path: str, iterations: int, base_url: str):
    """SOTA 설치/삭제 반복 테스트"""
    print("==================================================")
    print("      App Install/Uninstall Test Initialized")
    print("==================================================")
    print(f"  - App ID: {app_id}")
    print(f"  - App Version: {app_version}")
    print(f"  - App Path: {app_path}")
    print(f"  - Total Iterations: {iterations}")
    print("==================================================\n")

    pass_count = 0
    fail_count = 0

    for i in range(1, iterations + 1):
        print(f"---------- [ Iteration {i}/{iterations} ] ----------\n")
        
        # 1. 앱 설치 및 검증
        if not install_app(base_url, app_id, app_version, app_path):
            fail_count += 2
            print(f"[WARN] Iteration {i} failed at installation step. Proceeding to the next iteration.\n")
            continue
        
        time.sleep(VERIFICATION_DELAY_SECONDS)
        
        if verify_app_status(base_url, app_id, should_exist=True):
            pass_count += 1
        else:
            fail_count += 1
        print("-" * 20)

        # 2. 앱 삭제 및 검증
        if not uninstall_app(base_url, app_id):
            fail_count += 1
            print(f"[WARN] Iteration {i} failed at uninstallation step. Proceeding to the next iteration.\n")
            continue
        
        time.sleep(VERIFICATION_DELAY_SECONDS)

        if verify_app_status(base_url, app_id, should_exist=False):
            pass_count += 1
        else:
            fail_count += 1
        print("\n")

    print("\n==================================================")
    print("       App Install/Uninstall Test Summary")
    print("==================================================")
    print(f"  - Total Test Procedures: {iterations * 2} (Install/Uninstall)")
    print(f"  - Passed: {pass_count}")
    print(f"  - Failed: {fail_count}")
    print("==================================================")

    if fail_count > 0:
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="SOTA (Software Over-the-Air) Test")
    parser.add_argument("--app-path", type=str, required=True, help="File path for the application.")
    parser.add_argument("--app-id", type=str, required=True, help="Application ID (e.g., com.acp.test).")
    parser.add_argument("--app-version", type=str, required=True, help="Application version (e.g., 1.0.0).")
    parser.add_argument("--iterations", type=int, required=True, help="Number of test iterations.")
    parser.add_argument("--ip", type=str, required=True, help="IP address of the target device.")
    args = parser.parse_args()

    base_url = f"https://{args.ip}:9300/api/v1"
    run_sota_test(args.app_id, args.app_version, args.app_path, args.iterations, base_url)

if __name__ == "__main__":
    main()

앱 설치에 대해서는 이런식으로 코드를 작성했었는데, 가용 메모리 한계치에서 앱설치 후, 마지막 앱 기능 확인하고 삭제를 하기 위해서 우선 가용 메모리 한계치가 어디인지를 체크하는게 필요한데, 가용 메모리한계치를 어떻게 파악할 수 있을까 ? 
