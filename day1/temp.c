import subprocess

# wlan0_test.sh 파일 실행
script_path = './wlan0_test.sh'  # 스크립트 경로
interface = 'wlan0'  # 인터페이스 이름
log_file = 'last_log.txt'  # 로그를 저장할 파일 이름

try:
    # 스크립트를 실행하고 실시간으로 출력하면서 마지막 줄을 기록
    with open(log_file, 'w') as f:
        process = subprocess.Popen([script_path, interface], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        last_line = ""
        for line in process.stdout:
            print(line, end="")  # 터미널에 실시간 출력
            last_line = line.strip()  # 마지막 줄 갱신
            f.write(line)  # 로그 파일에 기록
        
        process.wait()  # 프로세스가 종료될 때까지 대기

    print(f"Last log line saved to {log_file}")

except FileNotFoundError:
    print("Script file not found. Please check the script path.")
