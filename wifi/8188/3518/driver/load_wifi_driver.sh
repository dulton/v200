#!/bin/sh
insmod cfg80211.ko&&insmod mac80211.ko&&insmod 8188eu.ko rtw_power_mgnt=0
sleep 3
if [ ! -f /config/wpa_supplicant.conf ]; then
./create_wifi_default_conf.sh
fi


