#!/bin/sh

kernel_release=`uname -r`
kernel_version=`expr "$kernel_release" : '\([0-9].[0-9]\+\)'`

# Bluetooth
sleep 1
lmp_version=`hciconfig -a | grep LMP | awk '{print $3}'`

# Write Secure Connections Host Support Command
if [ $kernel_version \> 3.14 ] && [ $lmp_version \> 4.1 ]; then
	btmgmt sc off 1>/dev/null
fi

sync
echo -e '\e[1;31m[ Booting Done ]\e[0m'
