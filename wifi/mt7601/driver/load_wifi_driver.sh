#!/bin/sh
insmod mt7601Usta.ko
usleep 1000
./create_wifi_default_conf.sh
