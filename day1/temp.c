#!/bin/bash

# 명령 인자 처리
while getopts "m:t:" opt; do
  case $opt in
    m) TARGET_MAC="$OPTARG" ;;
    t) TIMEOUT="$OPTARG" ;;
    \?) echo "Invalid option -$OPTARG" >&2 ;;
  esac
done

# 결과 파일 및 로그 파일 설정
RESULT_FILE="device_found.txt"
LOG_FILE="log.txt"
BLUETOOTH_LOG="bluetoothctl.log"  # bluetoothctl 출력을 저장할 파일

echo "false" > $RESULT_FILE
> $LOG_FILE
> $BLUETOOTH_LOG

echo "TARGET_MAC: $TARGET_MAC"

# 현재 BLE MAC 주소를 가져오는 함수
get_current_mac_address() {
    result=$(hcitool dev)
    echo "$result" | while read -r line; do
        if [[ $line == hci* ]]; then
            mac_address=$(echo $line | awk '{print $2}')
            echo "$mac_address"
            return
        fi
    done
    exit 1
}

# 현재 BLE MAC 주소를 가져오기
current_mac=$(get_current_mac_address)
echo "Current BLE MAC address is: $current_mac"

# 사용자에게 의도한 MAC 주소가 맞는지 묻기
while true; do
    echo "Is this the intended MAC address? (y/n): "
    read user_input
    user_input=$(echo "$user_input" | tr '[:upper:]' '[:lower:]')  # 소문자로 변환
    if [[ "$user_input" == "y" ]]; then
        echo "Proceeding with the intended MAC address."
        break
    elif [[ "$user_input" == "n" ]]; then
        echo "[ble] FAIL"
        exit 1
    else
        echo -e "\nInvalid input. Please enter 'y' or 'n'."
    fi
done

# 기본값 설정
TARGET_MAC=${TARGET_MAC:-"8A:88:4B:60:1F:FF"} # 찾고자 하는 MAC 주소
TIMEOUT=${TIMEOUT:-30} # 스캔할 최대 시간(초)

# 프로세스 종료를 처리하기 위한 트랩 설정
cleanup() {
  echo "Stopping scan..."
  echo "scan off" | bluetoothctl > /dev/null
  echo "exit" | bluetoothctl > /dev/null
}
trap cleanup EXIT

# bluetoothctl 실행 및 상호작용
{
  echo "scan on"
  sleep $TIMEOUT &
  wait
  cleanup
} | bluetoothctl > $BLUETOOTH_LOG 2>&1 &  # bluetoothctl 출력을 로그 파일에 리디렉션

# bluetoothctl PID 저장
bt_pid=$!

# 종료 시간을 설정합니다.
end_time=$((SECONDS + TIMEOUT))

while [ $SECONDS -lt $end_time ]; do
  # bluetoothctl에서 디바이스 목록을 가져옵니다.
  devices=$(bluetoothctl devices)

  # 디바이스 목록에서 TARGET_MAC이 있는지 확인합니다.
  if echo "$devices" | grep -q "$TARGET_MAC"; then
    echo "true" > $RESULT_FILE
    echo "Found target MAC: $TARGET_MAC"
    kill $bt_pid 2>/dev/null
    break
  fi
  sleep 1
done

# 모든 장치 제거
bluetoothctl devices | grep "Device" | while read -r line; do
  MAC=$(echo $line | awk '{print $2}')
  bluetoothctl remove $MAC
done

# 결과 출력
RESULT=$(cat $RESULT_FILE)
if [ "$RESULT" = "true" ]; then
  echo "[Ble] OK"
  echo "[Ble] OK"
  echo "[Ble] OK"
else
  echo "[Ble] FAIL"
fi
