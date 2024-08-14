import subprocess

# wlan0_test.sh 파일 실행
script_path = './wlan0_test.sh'  # 스크립트 경로
interface = 'wlan0'  # 인터페이스 이름
log_file = 'last_log.txt'  # 로그를 저장할 파일 이름

try:
    # 스크립트를 실행하고 터미널에 실시간 출력하면서 로그를 파일에 저장
    with open(log_file, 'w') as f:
        result = subprocess.run([script_path, interface], check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        # 출력된 모든 줄을 터미널에 출력하고 마지막 줄을 기록
        lines = result.stdout.splitlines()
        for line in lines:
            print(line)  # 터미널에 출력
            f.write(line + "\n")  # 로그 파일에 기록
        
        # 마지막 줄을 따로 저장
        last_line = lines[-1] if lines else ""
    
    print(f"Last log line saved to {log_file}")

except subprocess.CalledProcessError as e:
    print(f"Script failed with return code {e.returncode}")
    print(f"Output: {e.output}")
except FileNotFoundError:
    print("Script file not found. Please check the script path.")
