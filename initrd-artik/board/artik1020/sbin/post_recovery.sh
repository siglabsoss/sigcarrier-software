#!/bin/sh

SCRIPT_DIR=`dirname "$(readlink -f "$0")"`
ZIGBEE_VERSION=5.7.4

# Zigbee firmware check

CUR_DIR=$(pwd)
cd $SCRIPT_DIR
./802.15.4_setup.sh ncp-uart-rts-cts-use-with-serial-uart-btl-5.7.4.ebl 5.7.4 1 > /dev/null 2>&1
RET=$?
cd $CUR_DIR

if [ $RET == 0 ]; then
	echo "The zigbee fw version is the latest"
	exit 0
else
	echo "Invalid zigbee fw version"
	exit 1
fi
