#!/bin/sh

LED_RED=28
LED_BLUE=38
GPIO_PATH=/sys/class/gpio
SCRIPT_DIR=`dirname "$(readlink -f "$0")"`
ZIGBEE_VERSION=5.7.4

led_ctl()
{
	local led=$1
	local ctl=$2

	if ! [ -d $GPIO_PATH/gpio$led ]; then
		echo $led > $GPIO_PATH/export
	fi

	echo "out" > $GPIO_PATH/gpio$led/direction
	echo $ctl > $GPIO_PATH/gpio$led/value
}

led_ctl $LED_RED 0
led_ctl $LED_BLUE 1

# Zigbee firmware check

CUR_DIR=$(pwd)
cd $SCRIPT_DIR
./802.15.4_setup.sh ncp-uart-xon-xoff-use-with-serial-uart-btl-5.7.4.ebl 5.7.4 1 > /dev/null 2>&1
RET=$?
cd $CUR_DIR

if [ $RET == 0 ]; then
	echo "The zigbee fw version is the latest"
	exit 0
else
	echo "Invalid zigbee fw version"
	exit 1
fi
