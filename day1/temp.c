#!/bin/bash

# ./ble_test.sh -m "8A:88:4B:40:1D:64" -t 60

# 명령 인자 처리
while getopts "m:t:" opt; do
  case $opt in
    m) TARGET_MAC="$OPTARG" ;;
    t) TIMEOUT="$OPTARG" ;;
    \?) echo "Invalid option -$OPTARG" >&2 ;;
  esac
done

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

# 결과 파일 초기화
RESULT_FILE="device_found.txt"
echo "false" > $RESULT_FILE

# 스캔 결과를 저장할 파일 경로
LOG_FILE="log.txt"
> $LOG_FILE # 파일 초기화

# bluetoothctl을 사용하여 스캔을 시작
echo "Starting scan..."

# bluetoothctl 프로세스를 직접 제어
{
  # 블루투스 스캔 시작
  echo -e 'scan on\n'
  
  end_time=$((SECONDS + TIMEOUT))

  while [ $SECONDS -lt $end_time ]; do
    # bluetoothctl devices 명령어의 출력을 LOG_FILE에 저장하고 터미널에 출력
    bluetoothctl devices | tee -a $LOG_FILE

    # LOG_FILE에서 TARGET_MAC이 있는지 확인
    if grep -q "$TARGET_MAC" $LOG_FILE; then
      echo "true" > $RESULT_FILE
      echo "found target mac"

      # 블루투스 스캔 종료 및 bluetoothctl 종료
      echo -e 'scan off\n'
      echo -e 'exit\n'
      break
    fi
    sleep 1
  done

  # 만약 TIMEOUT까지 찾지 못했으면 스캔 종료
  echo -e 'scan off\n'
  echo -e 'exit\n'

} | bluetoothctl

# 모든 장치 제거
echo "Removing all devices..."
bluetoothctl devices | grep "Device" | while read -r line; do
  MAC=$(echo $line | awk '{print $2}')
  bluetoothctl remove $MAC
done

# RESULT_FILE의 값을 읽어 최종 결과 출력
RESULT=$(cat $RESULT_FILE)
if [ "$RESULT" = "true" ]; then
  echo "[Ble] OK"
  echo "[Ble] OK"
  echo "[Ble] OK"
else
  echo "[Ble] FAIL"
fi
