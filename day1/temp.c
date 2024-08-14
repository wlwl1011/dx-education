#!/bin/bash

INTERFACE=$1

ping_address="192.168.10.1"

# [Wifi mac 읽기 CMD] #

## 현재 WiFi MAC 주소를 출력
echo "[log]: read wifi mac address"

current_mac=$(cat /sys/class/net/wlan0/address)
echo " "
echo "----------------------------------------"
echo "| WiFi MAC Address |"
echo "----------------------------------------"
echo "| $current_mac |"
echo "----------------------------------------"
echo " "

# 사용자에게 확인 요청
while true; do
read -p "Is it right to intended wifi maccaddress ? (y/n):" user_input

if [ "$user_input" == "y" ]; then
echo "[log]: read mac address SUCCESS"
break
elif [ "$user_input" == "n" ]; then
echo "[wlan0]: FAIL (not intended wlan mac address)"
exit 1
else
echo "Wrong input. please input 'y' or 'n'."
fi
done



# [AP 연결] #

## Set a timeout duration (in seconds)
TIMEOUT_DURATION=60

## step 1: systemctl stop wpa_supplicant

echo "[log]: systemctl stop wpa_supplicant"
SECONDS=0
systemctl stop wpa_supplicant &
cmd_pid=$!
while kill -0 $cmd_pid 2> /dev/null; do
echo "[log] running systemctl stop wpa_supplicant ... in $SECONDS"
if [ $SECONDS -ge $TIMEOUT_DURATION ]; then
kill -9 $cmd_pid
wait $cmd_pid 2>/dev/null
echo "[log]: systemctl stop wpa_supplicant TIMEOUT"
break
fi
sleep 1
done
if [ $? -eq 0 ]; then
echo "[log]: systemctl stop wpa_supplicant SUCCESS"
else
echo "[wlan]: FAIL (systemctl stop wpa_supplicant SUCCESS)"
exit 1
fi

sleep 1

# step 2: ifconfig $INTERFACE up
echo "[log]: ifconfig $INTERFACE up"
SECONDS=0
ifconfig $INTERFACE up &
cmd_pid=$!
while kill -0 $cmd_pid 2> /dev/null; do
echo "[log] running ifconfig $INTERFACE up ... in $SECONDS"
if [ $SECONDS -ge $TIMEOUT_DURATION ]; then
kill -9 $cmd_pid
wait $cmd_pid 2>/dev/null
echo "[log]: ifconfig $INTERFACE up TIMEOUT"
break
fi
sleep 1
done

wait $cmd_pid
if [ $? -eq 0 ]; then
echo "[log]: ifconfig $INTERFACE up SUCCESS"
else
echo "[log]: ifconfig $INTERFACE up FAIL"
echo "[wlan]: FAIL (ifconfig $INTERFACE up)"
exit 1
fi

sleep 1

# 기존 wpa_supplicant 프로세스 종료 및 소켓 파일 삭제
if pgrep wpa_supplicant > /dev/null; then
echo "[log]: There is already a wpa_supplicant process."
echo "[log]: Terminating existing wpa_supplicant process."
wpa_pids=$(pgrep wpa_supplicant)
for pid in $wpa_pids; do
kill $pid
done
sleep 2
fi

if [ -e /var/run/wpa_supplicant/$INTERFACE ]; then
echo "[log] There is already socket file."
echo "[log]: Removing existing socket file."
rm /var/run/wpa_supplicant/$INTERFACE
fi


# step 3: wpa_supplicant -B -Dnl80211 -iwlan0 -c/etc/wpa_supplicant.conf
echo "[log]: wpa_supplicant -B -Dnl80211 -i$INTERFACE -c/etc/wpa_supplicant.conf"
wpa_supplicant -B -Dnl80211 -i$INTERFACE -c/etc/wpa_supplicant.conf &
cmd_pid=$!
while kill -0 $cmd_pid 2> /dev/null; do
echo "[log] running wpa_supplicant ... in $SECONDS"
if [ $SECONDS -ge $TIMEOUT_DURATION ]; then
kill -9 $cmd_pid
wait $cmd_pid 2>/dev/null
echo "[log]: wpa_supplicant TIMEOUT"
break
fi
sleep 1
done
wait $cmd_pid
if [ $? -eq 0 ]; then
echo "[log]: wpa_supplicant SUCCESS"
else
echo "[log]: wpa_supplicant FAIL"
echo "[wlan]: FAIL (wpa_supplicant -B -Dnl80211 -i$INTERFACE -c/etc/wpa_supplicant.conf)"
exit 1
fi



sleep 5
# step 4 : udhcpc -i $INTERFACE
echo "[log]: udhcpc -i $INTERFACE"
SECONDS=0
udhcpc -i $INTERFACE &
cmd_pid=$!
while kill -0 $cmd_pid 2> /dev/null; do
echo "[log] running udhcpc ... in $SECONDS"
if [ $SECONDS -ge $TIMEOUT_DURATION ]; then
echo "kill $cmd_pid"
kill -9 $cmd_pid
wait $cmd_pid 2>/dev/null
echo "[log]: udhcpc -i $INTERFACE TIMEOUT"
break
fi
sleep 1
done
wait $cmd_pid
if [ $? -eq 0 ]; then
echo "[log]: udhcpc -i $INTERFACE SUCCESS"
else
echo "[log]: udhcpc -i $INTERFACE FAIL"
dmesg > /lg_rw/dmesg_udhcpc_i_${INTERFACE}_fail.log
echo "[wlan]: FAIL (udhcpc -i $INTERFACE)"
exit 1
fi

sleep 5
echo "[log]: ping to $ping_address"
# # step 5 :Check if wlan0 is connected to the internet
SECONDS=0
ping -I $INTERFACE -c 5 $ping_address &> /dev/null &
cmd_pid=$!
while kill -0 $cmd_pid 2> /dev/null; do
echo "[log] running ping ... in $SECONDS"
if [ $SECONDS -ge $TIMEOUT_DURATION ]; then
kill -9 $cmd_pid
wait $cmd_pid 2>/dev/null
echo "[log]: ping TIMEOUT"
break
fi
sleep 1
done
wait $cmd_pid
if [ $? -eq 0 ]; then
echo "[wlan]: OK"
exit 1
else
echo "[wlan]: FAIL (ping)"
exit 1
fi

이렇게 작성된 파일을, 파이썬으로 실행시켰을 때 기존에 그냥 실행시켰을 때와 동일하게 작성되도록 파이썬 코드를 짜줘. 이 파일의 이름은 wlan0_test.sh 야.
