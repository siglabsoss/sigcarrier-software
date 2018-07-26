#!/bin/sh

if [ $# -ne 2 ]
then
echo '802.15.4_setup.sh [f/w file] [f/w version] [1:ver_check]'
exit 0
fi

FIRMWARE_FILE=$1
VERSION=$2
MAJOR_VERSION=${VERSION:0:5}
VER_CHECK=$3
ZIGBEE_VERSION_TOOL='./zigbee_version'
THREAD_VERSION_TOOL='./thread_version'
FIRMWARE_FLASHING_TOOL='./flash_firmware'

echo "Firmware file $FIRMWARE_FILE"
echo "Required version $VERSION"
echo "ZigBee version checking tool $ZIGBEE_VERSION_TOOL"
echo "Firmware flahsing tool $FIRMWARE_FLASHING_TOOL"

if [ ! -e "$FIRMWARE_FILE" ]; then
echo 'No firmware file'
exit 0
fi

if [ ! -e "$ZIGBEE_VERSION_TOOL" ]; then
echo 'No zigbee_version file'
exit 0
fi

if [ ! -e "$THREAD_VERSION_TOOL" ]; then
echo 'No thread_version file'
exit 0
fi

if [ ! -e "$FIRMWARE_FLASHING_TOOL" ]; then
echo 'No flash_firmware file'
exit 0
fi

ARTIK5=`cat /proc/cpuinfo | grep -i EXYNOS3`
ARTIK530=`cat /proc/cpuinfo | grep -i s5p4418`
ARTIK10=`cat /proc/cpuinfo | grep -i EXYNOS5`

if [ "$ARTIK5" != "" ]; then
        ZIGBEE_TTY="-p /dev/ttySAC1"
	THREAD_TTY="-u /dev/ttySAC1"
	VERSION=$MAJOR_VERSION
elif [ "$ARTIK530" != "" ]; then
        ZIGBEE_TTY="-p /dev/ttyAMA1"
	THREAD_TTY="-f x -u /dev/ttyAMA1"
elif [ "$ARTIK10" != "" ]; then
        ZIGBEE_TTY="-p /dev/ttySAC0"
        THREAD_TTY="-u /dev/ttySAC0"
	VERSION=$MAJOR_VERSION
else # ARTIK 710
        ZIGBEE_TTY="-n 1 -p /dev/ttySAC0"
        THREAD_TTY="-f x -u /dev/ttySAC0"
	VERSION=$MAJOR_VERSION
fi

echo 'Checking the current firmware'
ZIGBEE_FIRMWARE=$($ZIGBEE_VERSION_TOOL $ZIGBEE_TTY)
#echo "Testing...$ZIGBEE_FIRMWARE"
case "$ZIGBEE_FIRMWARE" in
*"ezsp ver"*)
echo 'Found ZigBee firmware'
	case "$ZIGBEE_FIRMWARE" in
	*"$VERSION"*)
	[ "$VER_CHECK" == "1" ] && exit 0
	echo 'Version matched, skip flashing';;
	*)
	[ "$VER_CHECK" == "1" ] && exit 1
	echo "Firmware v$VERSION flashing"
	exec $FIRMWARE_FLASHING_TOOL $ZIGBEE_TTY -f $FIRMWARE_FILE -n > /dev/null 2>&1
	echo "ZigBee Firmware v$VERSION has been flashed"
	;;
	esac
;;
*)
THREAD_FIRMWARE=$($THREAD_VERSION_TOOL $THREAD_TTY)
#echo "Testing...$THREAD_FIRMWARE"
	case "$THREAD_FIRMWARE" in
	*"Thread"*)
	[ "$VER_CHECK" == "1" ] && exit 0
	echo 'Found Thread firmware, skip flashing';;
	*)
	[ "$VER_CHECK" == "1" ] && exit 1
	THREAD_FIRMWARE=$($THREAD_VERSION_TOOL $THREAD_TTY)
	#echo "Testing2...$THREAD_FIRMWARE"
		case "$THREAD_FIRMWARE" in
		*"Thread"*)
		echo 'Found Thread firmware, skip flahsing';;
		*)
		echo 'Both ZigBee firmware and Thread firmware are not detected'
		echo "Firmware v$VERSION flashing"
		exec $FIRMWARE_FLASHING_TOOL $ZIGBEE_TTY -f $FIRMWARE_FILE -n > /dev/null 2>&1
		echo "ZigBee Firmware v$VERSION has been flashed"
		;;
		esac
	;;
	esac
;;
esac
