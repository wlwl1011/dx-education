import subprocess

# wlan0_test.sh 파일 실행
script_path = './wlan0_test.sh'  # 스크립트 경로
interface = 'wlan0'  # 인터페이스 이름

try:
    # 스크립트를 실행합니다. 인터페이스 이름을 인자로 전달합니다.
    result = subprocess.run([script_path, interface], check=True, text=True)
    print("Script executed successfully")
except subprocess.CalledProcessError as e:
    print(f"Script failed with return code {e.returncode}")
    print(f"Output: {e.output}")
except FileNotFoundError:
    print("Script file not found. Please check the script path.")
