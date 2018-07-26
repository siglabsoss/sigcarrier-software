#!/bin/sh

SCRIPT_PATH="$(dirname $(readlink -f "$0"))"

SD_ROOT_DEV=/dev/mmcblk1p3

# Check if there is OTA feature or not.
if [ -n "`cat /proc/cmdline |grep ota`" ]; then
	EMMC_ROOT_DEV=/dev/mmcblk0p7
else
	EMMC_ROOT_DEV=/dev/mmcblk0p3
fi

SD_MNT=/root/sd_root

mkdir -p $SD_MNT
mkdir /mnt

set_performance_mode()
{
	# disable hotplug
	if [ -e /sys/devices/system/cpu/hotplug/force_hstate ]; then
		echo 0 > /sys/devices/system/cpu/hotplug/force_hstate
	fi

	# set performance mode
	if [ -e /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ]; then
		echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
	fi

	# set performance mode for HMP
	if [ -e /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor ]; then
		echo performance > /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor
	fi

	# set devfreq to maximum for nexell
	if [ -e /sys/class/devfreq/nx-devfreq/min_freq ]; then
		echo 400000 > /sys/class/devfreq/nx-devfreq/min_freq
	fi
}

fuse_rootfs_partition()
{
	mkfs.ext4 -F $EMMC_ROOT_DEV -L rootfs > /dev/null 2>&1

	mount -t ext4 $EMMC_ROOT_DEV /mnt > /dev/null 2>&1
	pv $SD_MNT/rootfs.tar.gz | tar zxf - -C /mnt > /dev/null 2>&1
	cp $SD_MNT/artik_release /mnt/etc > /dev/null 2>&1
	sync; sync; sync
}

set_performance_mode

sync
mdev -s

mount -t ext4 $SD_ROOT_DEV $SD_MNT

[ -e $SCRIPT_PATH/pre_recovery.sh ] && $SCRIPT_PATH/pre_recovery.sh
echo "Please wait until the fusing has been finished"

fuse_rootfs_partition

RET=$?

if [ -e $SCRIPT_PATH/post_recovery.sh ]; then
	$SCRIPT_PATH/post_recovery.sh
	RET=$?
fi

if [ $RET == 0 ]; then
	echo "Fusing is done."
	echo "Please turn off the board and convert to eMMC boot mode"
else
	echo "Fusing has been failed."
	echo "Please check your fimrware file and try again"
fi
