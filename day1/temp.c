#!/bin/bash



# 사용법 출력 함수

usage() {
    echo "Usage: $0 <mount_point>"
    exit 1
}



# 인자 개수 확인

if [ "$#" -ne 1 ]; then
    usage
fi



# USB 마운트 포인트

MOUNT_POINT="$1"

TEST_DIR="${MOUNT_POINT}/fct"

TEST_FILE="${TEST_DIR}/usb.test"



# USB 장치 이름 확인

USB_DEV_NAME=$(lsblk -o NAME,TYPE | grep 'disk' | grep -o '^sd[a-z]')

if [ -z "$USB_DEV_NAME" ]; then
    echo "[USB]: Fail - No USB device found"
    exit 1
fi



# 디스크와 파티션 이름 확인

PARTITIONS=$(ls /dev/${USB_DEV_NAME}*)



# 마운트 시도

MOUNT_SUCCESS=0

for PARTITION in $PARTITIONS; do
    if mount "$PARTITION" "$MOUNT_POINT"; then
        MOUNT_SUCCESS=1
        USB_DEV_PATH="$PARTITION"
        break
    fi
done



if [ $MOUNT_SUCCESS -ne 1 ]; then
    echo "[USB]: Fail - Could not mount any USB partition"
    exit 1
fi



# USB가 마운트되었는지 확인

if ! df | grep -q "$USB_DEV_PATH"; then
    echo "[USB]: Fail - USB device not mounted"
    exit 1
fi



# 테스트 디렉토리 생성 (이미 존재하면 삭제 후 재생성)

if ! mkdir -p "$TEST_DIR"; then
    if ! rm -rf "$TEST_DIR"; then
        echo "[USB]: Fail - Could not remove existing test directory"
        exit 1
    fi

    if ! mkdir -p "$TEST_DIR"; then
        echo "[USB]: Fail - Could not recreate test directory"
        exit 1
    fi

fi



# 테스트 파일 작성 및 내용 확인

echo "usb_test" > "$TEST_FILE"

if [ $? -eq 0 ]; then

    read content < "$TEST_FILE"
    if [ "$content" == "usb_test" ]; then
        echo "USB write file and verified"

    else
        echo "[USB]: Fail - File content verification failed"
        exit 1
    fi

else
    echo "[USB]: Fail - Could not write test file"
    exit 1
fi



# 언마운트
if ! umount "$MOUNT_POINT"; then
    echo "[USB]: Fail - Could not unmount USB device"
    exit 1
fi



# 테스트 디렉토리 삭제
if ! rm -rf "$TEST_DIR"; then
    echo "[USB]: Fail - Could not remove test directory after unmount"
    exit 1
fi

echo "[USB]: OK"
