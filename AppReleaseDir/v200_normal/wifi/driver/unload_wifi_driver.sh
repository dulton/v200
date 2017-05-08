#!/bin/sh
killall  wpa_supplicant
sleep 1
rmmod 8188eu;rmmod mac80211;rmmod cfg80211;
sleep 1
