#!/bin/sh
insmod compat.ko&&insmod cfg80211.ko&&insmod mac80211.ko&&insmod ath.ko&&insmod ath9k_hw.ko
insmod ath9k_common.ko&&insmod ath9k_htc.ko
sleep 3
./create_wifi_default_conf.sh


