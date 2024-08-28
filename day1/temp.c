import subprocess
import os
import shutil

# USB 장치가 마운트된 위치를 탐색하는 함수
def find_usb_mount_point():
    try:
        mount_output = subprocess.run("lsblk -o MOUNTPOINT,NAME,FSTYPE | grep exfat", shell=True, capture_output=True, text=True).stdout.strip()
        if mount_output:
            # exFAT 파일 시스템이 포함된 라인을 찾음
            lines = mount_output.splitlines()
            for line in lines:
                if 'exfat' in line:
                    mount_point = line.split()[0]  # 마운트 위치
                    return mount_point
    except Exception as e:
        print(f"Failed to find USB mount point: {e}")
    return None

# USB 마운트 포인트 찾기
usb_mount_point = find_usb_mount_point()

if usb_mount_point:
    # USB 로그 파일 경로 생성
    usb_full_log_path = os.path.join(usb_mount_point, usb_log_file_path)

    try:
        # 로그 파일을 USB로 복사
        shutil.copy(log_file_path, usb_full_log_path)
        print(f"Log file successfully copied to USB: {usb_full_log_path}")
    except Exception as e:
        print(f"Failed to copy log file to USB: {e}")
else:
    print("USB device not found or not mounted.")
