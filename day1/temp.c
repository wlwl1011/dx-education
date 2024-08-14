import subprocess
import os
import pty

# wlan0_test.sh 파일 실행
script_path = './wlan0_test.sh'  # 스크립트 경로
interface = 'wlan0'  # 인터페이스 이름
log_file = 'last_log.txt'  # 로그를 저장할 파일 이름

def run_script_with_pty(script_path, interface):
    master_fd, slave_fd = pty.openpty()  # 의사 터미널을 엽니다.

    try:
        process = subprocess.Popen([script_path, interface], stdin=slave_fd, stdout=slave_fd, stderr=slave_fd, text=True)
        os.close(slave_fd)  # 더 이상 필요하지 않으므로 닫습니다.

        last_line = ""
        with open(log_file, 'w') as f:
            while True:
                try:
                    output = os.read(master_fd, 1024).decode()  # 출력 읽기
                    if not output:
                        break
                    print(output, end='')  # 실시간으로 터미널에 출력
                    f.write(output)  # 로그 파일에 기록

                    # 로그의 마지막 줄 갱신
                    last_line = output.strip()
                except OSError:
                    break

        process.wait()  # 프로세스가 종료될 때까지 대기

        print(f"\nLast log line saved to {log_file}")
        return last_line

    finally:
        os.close(master_fd)  # 마스터 파일 디스크립터를 닫습니다.

try:
    last_log_line = run_script_with_pty(script_path, interface)
except FileNotFoundError:
    print("Script file not found. Please check the script path.")
