import subprocess

# wlan0_test.sh 파일 실행
script_path = './wlan0_test.sh'  # 스크립트 경로
interface = 'wlan0'  # 인터페이스 이름
log_file = 'last_log.txt'  # 로그를 저장할 파일 이름

try:
    # 스크립트를 실행하고 stdout을 캡처합니다.
    result = subprocess.run([script_path, interface], check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    
    # 캡처된 로그를 줄 단위로 나눕니다.
    output_lines = result.stdout.splitlines()
    
    # 마지막 10줄을 저장합니다.
    last_lines = output_lines[-10:]  # 필요에 따라 숫자를 조정하여 더 많은 줄을 가져올 수 있습니다.
    
    with open(log_file, 'w') as f:
        f.write('\n'.join(last_lines))
    
    print(f"Last log lines saved to {log_file}")

except subprocess.CalledProcessError as e:
    print(f"Script failed with return code {e.returncode}")
    print(f"Output: {e.output}")
except FileNotFoundError:
    print("Script file not found. Please check the script path.")
