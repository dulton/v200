#!/bin/sh

#config wlan ip
ifconfig wlan0 1.1.1.101 up

#connect wifi
#./iwconfig wlan0 essid "ivan_t2"
killall -9 wpa_supplicant 
./wpa_supplicant -Dwext -iwlan0 -c./wpa_supplicant.conf& 


