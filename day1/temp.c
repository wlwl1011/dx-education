#!/usr/bin/env python3

import subprocess
import sys
import select
import os
from datetime import datetime
from concurrent.futures import ThreadPoolExecutor, as_completed
import yaml

# YAML 파일 읽기
with open('/lg_rw/fct_test/cfg.yml', 'r') as file:
    config = yaml.safe_load(file)

# global 설정 읽기
global_config = config['global']

# WiFi 설정을 wpa_supplicant.conf 파일에 작성
wifi_config = config['wifi']
if wifi_config['enable']:
    wpa_supplicant_conf = f"""
ctrl_interface=/var/run/wpa_supplicant
ctrl_interface_group=0
update_config=1
network={{
    ssid="{wifi_config['ssid']}"
    psk="{wifi_config['password']}"
    key_mgmt=WPA-PSK
}}
"""
    with open('/lg_rw/fct_test/wpa_supplicant.conf', 'w') as wpa_file:
        wpa_file.write(wpa_supplicant_conf)

    # 네트워크 인터페이스 설정
    subprocess.run("dmesg -n 1", shell=True)
    subprocess.run("mount -o rw,remount /", shell=True)
    subprocess.run("ifconfig wlan0 up", shell=True)
    # wpa_supplicant.conf 파일 복사 및 권한 설정
    subprocess.run("cp -f /lg_rw/fct_test/wpa_supplicant.conf /etc/wpa_supplicant.conf", shell=True)
    subprocess.run("chmod +x /etc/wpa_supplicant.conf", shell=True)
    # 인터넷 연결 확인 및 시간 동기화
    # subprocess.run("wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf -q &> /dev/null", shell=True)
    # subprocess.run("dhclient wlan0", shell=True)

# NTP 동기화 상태 확인
# ntp_status = subprocess.run("timedatectl show -p NTPSynchronized --value", shell=True, capture_output=True, text=True).stdout.strip()

# # NTP 동기화가 되어 있지 않으면 시간 동기화 시도
# if ntp_status != "yes":
#     print("NTP not synchronized. Synchronizing time...")
#     subprocess.run("ntpdate -u pool.ntp.org", shell=True)
#     ntp_status = subprocess.run("timedatectl show -p NTPSynchronized --value", shell=True, capture_output=True, text=True).stdout.strip()
#     if ntp_status != "yes":
#         print("Failed to synchronize time.")

# 로그 파일 설정
log_file_path = '/lg_rw/fct_test/fct_test.log'

# 로그 파일에 시간 기록
with open(log_file_path, 'a') as log_file:
    log_file.write("###############################################################\n")
    # log_file.write(f"Test started at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
    # if ntp_status != "yes":
    #     log_file.write("Failed to synchronize time.\n")

# 테스트 항목들
test_items = {
    'wifi': 'wlan0_test.sh',
    'gpio': 'gpio_test.sh',
    'ble': 'ble_test.sh',
    'spi': 'spi_test_dq1.sh',
    'i2c': 'i2c_test_total.sh',
    'uart': '485_test.sh',
    'pwm': 'pwm_test_total.sh',
    'usb': 'usb_test.sh',
    'LCD': 'lcd_test.sh'
}

# 실시간 출력을 처리하는 함수
def run_test_script(script, args):
    print(f"Running {script} {args}...", flush=True)
    process = subprocess.Popen(
        f"{script} {args}",
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True,  # 텍스트 모드로 스트림 처리
        bufsize=1  # 라인 버퍼링
    )
    log_lines = []
    last_line = ""

    while True:
        try:
            output = process.stdout.readline()
            if output:
                print(output.strip(), flush=True)  # flush=True로 강제 플러시
                log_lines.append(output.strip())
                last_line = output.strip()
            if process.poll() is not None:
                break
        except UnicodeDecodeError:
            print("result message include UnicodeDecodeError. Ignoring the line.")
    process.wait()  # Add this line
    return process.returncode, log_lines


#실시간 출력을 처리하는 함수
def run_test_script_spi(script, args):
    print(f"Running {script} {args.strip()}...", flush=True)  # strip args to remove trailing spaces
    process = subprocess.run(f"{script} {args.strip()}", shell=True, capture_output=True, text=True)  # strip args to remove trailing spaces
    log_lines = []

    try:
        output = process.stdout.split('\n')  # split the output into lines
        for line in output:
            line = line.strip()  # strip line to remove trailing spaces
            print(line, flush=True)  # flush=True로 강제 플러시
            log_lines.append(line)
    except UnicodeDecodeError:
        print("result message include UnicodeDecodeError. Ignoring the line.")
    return process.returncode, log_lines

# 테스트 실행 및 결과 저장
test_results = {}

def execute_test(item, script):
    if config[item]['enable']:
        test_name = item
        repeat = config[item]['repeat']
        print("###############################################################")
        print(f"Running {test_name}...", flush=True)

        for _ in range(repeat):
            if item == 'uart':
                args = f"{config['uart']['boadrate']} {config['uart']['data']}"
            elif item == 'i2c':
                args = f"{config['i2c']['speed']} {config['i2c']['data']}"
            elif item == 'spi':
                args = f"{config['spi']['data']} {config['spi']['speed']}"
            elif item == 'wifi':
                args = f"{wifi_config['name']}"
            elif item == 'ble':
                args = f"-m \"{config['ble']['mac']}\""
            elif item == 'usb':
                args = f"{config['usb']['file_path']}"
            else:
                args = ""

            if item == "spi":
                result_code, log_lines = run_test_script_spi(f"/lg_rw/fct_test/{script}", args)
                last_line = log_lines[-2] if log_lines else "No output" # 공백이 이 추가되어 -2로 결과 반환
            else:
                result_code, log_lines = run_test_script(f"/lg_rw/fct_test/{script}", args)
                last_line = log_lines[-1] if log_lines else "No output"


            print(last_line, flush=True)

            if "OK" in last_line:
                test_results[test_name] = last_line
                break
        else:
            # If the loop completes without a break, record the last result
            test_results[test_name] = last_line

# 병렬 실행 여부 확인
parallel = config.get('global', {}).get('parallel', False)
if parallel:
    with ThreadPoolExecutor() as executor:
        futures = {executor.submit(execute_test, item, script): item for item, script in test_items.items()}
        for future in as_completed(futures):
            item = futures[future]
            try:
                future.result()
            except Exception as exc:
                print(f"{item} generated an exception: {exc}")
else:
    for item, script in test_items.items():
        execute_test(item, script)


# 테스트 결과 확인 및 출력
all_tests_passed = True
failed_count = 0
total_count = 0
print("-" * 40, flush=True)
print("-" * 40, flush=True)
with open(log_file_path, 'a') as log_file:
    for test_name, result in test_results.items():
        total_count += 1
        print(result, flush=True)
        log_file.write(f"{test_name}: {result}\n")
        if "OK" not in result:
            all_tests_passed = False
            failed_count += 1
    print("-" * 40, flush=True)
    print("-" * 40, flush=True)
    log_file.write("-" * 40 + "\n")

# 전체 테스트 결과 출력
with open(log_file_path, 'a') as log_file:
    if all_tests_passed:
        print("[ALL TESTS]: OK", flush=True)
        log_file.write("[ALL TESTS]: OK\n")
    else:
        print(f"[ALL TESTS]: {failed_count} out of {total_count} tests failed", flush=True)
        log_file.write(f"[ALL TESTS]: {failed_count} out of {total_count} tests failed\n")

# Autostart 설정
if global_config.get('autostart', False):
    service_name = "my_dq1_app.service"
    service_path = f"/usr/lib/systemd/system/{service_name}"
    # 서비스가 이미 활성화되어 있는지 확인
    is_enabled = subprocess.run(f"systemctl is-enabled {service_name}", shell=True, capture_output=True, text=True).stdout.strip()
    if is_enabled != "enabled":
        # 디렉토리가 없으면 생성
        os.makedirs(os.path.dirname(service_path), exist_ok=True)
        # 서비스 파일 복사 및 권한 설정
        subprocess.run(f"cp -f /lg_rw/fct_test/{service_name} {service_path}", shell=True)
        subprocess.run("systemctl daemon-reload", shell=True)
        subprocess.run(f"systemctl start {service_name}", shell=True)
        subprocess.run(f"systemctl enable {service_name}", shell=True)
