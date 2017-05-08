#!/bin/sh

wifi_dir=/config

wpa_supplicant=$wifi_dir/wpa_supplicant.conf

if [ ! -e $wpa_supplicant ]; then
    echo "ctrl_interface=/config/run/wpa_supplicant" > $wpa_supplicant
    echo "ctrl_interface_group=0" >> $wpa_supplicant
    echo "update_config=1" >> $wpa_supplicant
    echo "network={" >> $wpa_supplicant
    echo "  ssid=\"Zmodo_WiFi_NVR\"" >> $wpa_supplicant
    echo "  key_mgmt=WPA-PSK" >> $wpa_supplicant
    echo "  proto=WPA RSN" >> $wpa_supplicant
    echo "  pairwise=CCMP TKIP" >> $wpa_supplicant
    echo "  group=CCMP TKIP" >> $wpa_supplicant
    echo "  psk=\"12345678\"" >> $wpa_supplicant
    echo "}" >> $wpa_supplicant
fi
chmod 755 $wpa_supplicant