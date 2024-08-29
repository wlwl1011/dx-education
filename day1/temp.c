#!/usr/bin/env python3

import subprocess
import os
from datetime import datetime
from concurrent.futures import ThreadPoolExecutor, as_completed
import yaml
import shutil
import hashlib


# 사용자에게 날짜 입력을 받아 확인
def get_log_datetime():
    while True:
        datetime_input = input("Enter the log date and time (YYYY-MM-DD HH:MM, leave empty for current date and time): ")
        if not datetime_input:
            log_datetime = datetime.now().strftime('%Y-%m-%d %H:%M')
            break
        try:
            log_datetime = datetime.strptime(datetime_input, '%Y-%m-%d %H:%M').strftime('%Y-%m-%d %H:%M')
            confirm = input(f"Confirm log date and time as {log_datetime}? (y/n): ").strip().lower()
            if confirm == 'y':
                break
        except ValueError:
            print("Invalid date and time format. Please use YYYY-MM-DD HH:MM.")
    return log_datetime

log_datetime = get_log_datetime()
print(f"Log date and time: {log_datetime}")

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
def get_mac_address(interface,target):
    try:
        mac = subprocess.run(f"cat /sys/class/{target}/{interface}/address", shell=True, capture_output=True, text=True).stdout.strip()
        mac = mac.replace(":", "")  # 콜론 제거
    except Exception as e:
        mac = "unknown"
    return mac

def get_current_ble_mac_address():
    try:
        result = subprocess.run("hcitool dev", shell=True, capture_output=True, text=True).stdout.strip()
        
        for line in result.splitlines():
            if "hci" in line:
                mac_address = line.split()[1]
                return mac_address.replace(":", "") 
    except Exception as e:
        print(f"Failed to get current BLE MAC address: {e}")
    return "unknown"


wifi_mac = get_mac_address('wlan0','net')
ble_mac = get_current_ble_mac_address()
# print(ble_mac)

# 로그 파일 설정
log_file_path = '/lg_rw/fct_test/fct_test.log'
terminal_log_file_path = '/lg_rw/fct_test/fct_test_terminal.log'
usb_log_file_path = f"fct_test_{wifi_mac}_{ble_mac}.log"

# 로그 파일에 날짜 기록
with open(log_file_path, 'a') as log_file:
    log_file.write("###############################################################\n")
    log_file.write(f"Test started at: {log_datetime}\n")

# 터미널 로그 파일에 날짜 기록
with open(terminal_log_file_path, 'a') as log_file:
    log_file.write("###############################################################\n")
    log_file.write(f"Test started at: {log_datetime}\n")


# 테스트 항목들
test_items = {
    'LCD': 'lcd_test.sh',
    'wifi': 'wlan0_test.sh',
    'gpio': 'gpio_test.sh',
    'ble': 'ble_test.sh',
    'spi': 'spi_test_dq1.sh',
    'i2c': 'i2c_test_total.sh',
    'uart': '485_test.sh',
    'pwm': 'pwm_test_total.sh',
    'usb': 'usb_test.sh',
}

# 실시간 출력을 처리하는 함수
def run_test_script(script, args):
    #print(f"Running {script} {args}...", flush=True)
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

            #print(last_line, flush=True)

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

def find_usb_device():
    for attempt in range(3):
        try:
            # /dev 디렉토리에서 exfat 파일 시스템을 가진 파티션을 찾음
            lsblk_output = subprocess.run("lsblk -o NAME,FSTYPE", shell=True, capture_output=True, text=True).stdout.strip()
            if lsblk_output:
                lines = lsblk_output.splitlines()
                for line in lines:
                    parts = line.split()
                    if len(parts) >= 2 and 'exfat' in parts[1]:  # exFAT 파일 시스템이 포함된 라인을 찾음
                        device_name = parts[0].strip('`-')  # 디바이스 이름에서 `-` 문자를 제거
                        return f"/dev/{device_name}"
        except Exception as e:
            print(f"Attempt {attempt + 1}: Failed to find USB device: {e}")
    return None

def is_mounted(device_name, mount_point):
    try:
        # mount 명령어를 사용하여 현재 마운트된 디바이스 목록을 확인
        result = subprocess.run("mount", shell=True, capture_output=True, text=True).stdout.strip()
        for line in result.splitlines():
            if device_name in line and mount_point in line:
                return True
    except Exception as e:
        print(f"Failed to check if device is mounted: {e}")
    return False

def mount_usb(device_name, mount_point):
    for attempt in range(3):
        try:
            if not mount_point:
                mount_point = "/lg_rw/fct_test/result"
                os.makedirs(mount_point, exist_ok=True)
            
            # 디바이스가 이미 마운트되어 있는지 확인
            if is_mounted(device_name, mount_point):
                print(f"{device_name} is already mounted on {mount_point}")
                return mount_point
            
            # 마운트 포인트가 존재하지 않으면 생성
            if not os.path.exists(mount_point):
                subprocess.run(f"mkdir -p {mount_point}", shell=True, check=True)
            
            # 디바이스 마운트
            subprocess.run(f"mount {device_name} {mount_point}", shell=True, check=True)
            return mount_point
        except Exception as e:
            print(f"Attempt {attempt + 1}: Failed to mount USB device: {e}")
    return None

def unmount_usb(mount_point):
    for attempt in range(3):
        try:
            subprocess.run(f"umount {mount_point}", shell=True, check=True)
            print(f"Successfully unmounted {mount_point}")
            return True
        except Exception as e:
            print(f"Attempt {attempt + 1}: Failed to unmount USB device: {e}")
    return False

def compare_files(file1, file2):
    try:
        with open(file1, 'r') as f1, open(file2, 'r') as f2:
            for line1, line2 in zip(f1, f2):
                if line1 != line2:
                    return False
        return True
    except Exception as e:
        print(f"Failed to compare files: {e}")
        return False

def copy_log_file(src, dst):
    for attempt in range(3):
        try:
            shutil.copy(src, dst)
            print(f"Log file copied to USB: {dst}")
            if compare_files(src, dst):
                print(f"Log file successfully verified: {dst}")
                return True
            else:
                print(f"Verification failed for {dst}. Retrying...")
        except Exception as e:
            print(f"Attempt {attempt + 1}: Failed to copy log file to USB: {e}")
    return False

# USB 장치 찾기
device_name = find_usb_device()
print(f"USB Device Name: {device_name}")  # 디버깅을 위해 추가

if device_name:
    # USB 장치 마운트
    mount_point = mount_usb(device_name, "/lg_rw/fct_test/result")
    if mount_point:
        # 로그 파일 경로 생성
        usb_full_log_path = os.path.join(mount_point, usb_log_file_path)
        print(f"USB Full Log Path: {usb_full_log_path}")  # 디버깅을 위해 추가

        # 로그 파일을 USB로 복사 및 검증
        if copy_log_file(log_file_path, usb_full_log_path):
            # USB 장치 언마운트
            if not unmount_usb(mount_point):
                print("Failed to unmount USB device after 3 attempts.")
        else:
            print("Failed to copy and verify log file to USB after 3 attempts.")
    else:
        print("Failed to mount USB device after 3 attempts.")
else:
    print("USB device not found after 3 attempts.")
