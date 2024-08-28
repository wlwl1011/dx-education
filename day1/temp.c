#!/usr/bin/env python3

import subprocess
import os
from datetime import datetime
import yaml

# 사용자에게 날짜 입력을 받아 확인
def get_log_date():
    while True:
        date_input = input("Enter the log date (YYYY-MM-DD, leave empty for today's date): ")
        if not date_input:
            log_date = datetime.now().strftime('%Y-%m-%d')
            break
        try:
            log_date = datetime.strptime(date_input, '%Y-%m-%d').strftime('%Y-%m-%d')
            confirm = input(f"Confirm log date as {log_date}? (y/n): ").strip().lower()
            if confirm == 'y':
                break
        except ValueError:
            print("Invalid date format. Please use YYYY-MM-DD.")
    return log_date

log_date = get_log_date()

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

# WiFi 및 BLE MAC 주소 읽기
def get_mac_address(interface):
    try:
        mac = subprocess.run(f"cat /sys/class/net/{interface}/address", shell=True, capture_output=True, text=True).stdout.strip()
    except Exception as e:
        mac = "unknown"
    return mac

wifi_mac = get_mac_address('wlan0')
ble_mac = get_mac_address('hci0')  # hci0은 BLE 인터페이스로 가정

# 로그 파일 설정
log_file_path = '/lg_rw/fct_test/fct_test.log'
usb_log_file_name = f"fct_test_{wifi_mac}_{ble_mac}.log"
usb_log_file_path = None

# USB 마운트 경로를 찾고 로그 파일을 저장하는 함수
def mount_usb_and_save_log():
    global usb_log_file_path
    mount_point = "/mnt/usb_test"  # USB를 마운트할 임시 경로
    os.makedirs(mount_point, exist_ok=True)

    # USB 장치 이름 확인
    usb_dev_name = subprocess.run("lsblk -o NAME,TYPE | grep 'disk' | grep -o '^sd[a-z]'", shell=True, capture_output=True, text=True).stdout.strip()
    if not usb_dev_name:
        print("[USB]: Fail - No USB device found")
        return False

    # USB 파티션 마운트 시도
    partitions = [f"/dev/{usb_dev_name}{p}" for p in "1 2 3 4".split()]
    mount_success = False
    for partition in partitions:
        result = subprocess.run(f"mount {partition} {mount_point}", shell=True)
        if result.returncode == 0:
            mount_success = True
            usb_dev_path = partition
            break

    if not mount_success:
        print("[USB]: Fail - Could not mount any USB partition")
        return False

    usb_log_file_path = os.path.join(mount_point, usb_log_file_name)

    # 로그 파일 작성
    try:
        with open(usb_log_file_path, 'w') as usb_log_file:
            usb_log_file.write("###############################################################\n")
            usb_log_file.write(f"Test started at: {log_date} {datetime.now().strftime('%H:%M:%S')}\n")
        print(f"[USB]: Log file saved at {usb_log_file_path}")
    except Exception as e:
        print(f"[USB]: Fail - Could not write log file: {e}")
        return False

    # USB 언마운트
    subprocess.run(f"umount {mount_point}", shell=True)
    os.rmdir(mount_point)
    return True

if not mount_usb_and_save_log():
    print("USB logging failed. Logs will only be saved locally.")

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

# 테스트 결과 확인 및 출력
all_tests_passed = True
failed_count = 0
total_count = 0
print("-" * 40, flush=True)
print("-" * 40, flush=True)
with open(log_file_path, 'a') as log_file:
    if usb_log_file_path:
        with open(usb_log_file_path, 'a') as usb_log_file:
            for test_name, result in test_results.items():
                total_count += 1
                print(result, flush=True)
                log_file.write(f"{test_name}: {result}\n")
                usb_log_file.write(f"{test_name}: {result}\n")
            print("-" * 40, flush=True)
            print("-" * 40, flush=True)
            log_file.write("-" * 40 + "\n")
            usb_log_file.write("-" * 40 + "\n")

# 전체 테스트 결과 출력
with open(log_file_path, 'a') as log_file:
    if usb_log_file_path:
        with open(usb_log_file_path, 'a') as usb_log_file:
            if all_tests_passed:
                print("[ALL TESTS]: OK", flush=True)
                log_file.write("[ALL TESTS]: OK\n")
                usb_log_file.write("[ALL TESTS]: OK\n")
            else:
                print(f"[ALL TESTS]: {failed_count} out of {total_count} tests failed", flush=True)
                log_file.write(f"[ALL TESTS]: {failed_count} out of {total_count} tests failed\n")
                usb_log_file.write(f"[ALL TESTS]: {failed_count} out of {total_count} tests failed\n")

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
