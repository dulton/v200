#telnetd &
cp /app/tool/* /tmp
cp /app/message /tmp
mkdir -p /config/run/wpa_supplicant/
mount -t tmpfs tmfs /system 
cd /app/wifi/driver/ 
./load_wifi_driver.sh
cd /app/hi3518
./load3518 -i
cd /tmp
./message&
cd /app
./App3518&

