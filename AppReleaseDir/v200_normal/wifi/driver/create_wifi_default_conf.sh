#!/bin/sh

wifi_dir=/config

wpa_supplicant=$wifi_dir/wpa_supplicant.conf

echo "update_config=1" >> $wpa_supplicant

echo "ctrl_interface=/config/run/wpa_supplicant" >> $wpa_supplicant
chmod 755 $wpa_supplicant