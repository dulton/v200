#!/bin/sh
killall  wpa_supplicant
sleep 1
rmmod ath9k_htc;rmmod ath9k_common;rmmod ath9k_hw;rmmod ath;rmmod mac80211;rmmod cfg80211;rmmod compat_firmware_class;rmmod compat;
sleep 1
