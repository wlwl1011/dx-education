import subprocess

# wlan0_test.sh 파일 실행
script_path = './wlan0_test.sh'  # 스크립트 경로
interface = 'wlan0'  # 인터페이스 이름
log_file = 'last_log.txt'  # 로그를 저장할 파일 이름

try:
    process = subprocess.Popen(
        [script_path, interface],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1  # Line buffering
    )

    last_line = ""
    with open(log_file, 'w') as f:
        while True:
            output = process.stdout.readline()
            if output == '' and process.poll() is not None:
                break
            if output:
                print(output, end='')  # 터미널에 실시간 출력
                last_line = output.strip()  # 마지막 줄 갱신
                f.write(output)  # 로그 파일에 기록

        process.wait()  # 프로세스가 종료될 때까지 대기

    # 마지막 줄을 파일에 저장
    with open(log_file, 'a') as f:
        f.write(f"\nLast line: {last_line}\n")

    print(f"\nLast log line saved to {log_file}")

except subprocess.CalledProcessError as e:
    print(f"Script failed with return code {e.returncode}")
except FileNotFoundError:
    print("Script file not found. Please check the script path.")
