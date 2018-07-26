#!/bin/bash

set -e

PREBUILT_UINITRD=`pwd`/prebuilt/uInitrd
OUTPUT_DIR=`pwd`/output
SCRIPT_DIR=`pwd`/scripts
BOARD_DIR=`pwd`/board
TARGET=

usage()
{
	echo "usage: convert_ext2fs.sh [options]"
	echo "-h		Print this help message"
	echo "-b [board]	Target board(artik520, artik1020, artik530, artik710)"
}

parse_options()
{
	TEMP=`getopt -o "h:b:" -- "$@"`
	eval set -- "$TEMP"
	case "$1" in
		-b ) TARGET=$2;;
		-h | * ) usage; exit 1 ;;
	esac
}

parse_options $@

[ -d $OUTPUT_DIR/sys_root ] || mkdir $OUTPUT_DIR/sys_root

pushd $OUTPUT_DIR

dd if=/dev/zero of=ramdisk bs=1M count=16
mkfs.ext2 -F ramdisk

dd if=$PREBUILT_UINITRD of=initrd.gz bs=64 skip=1

sudo mount -o loop ramdisk $OUTPUT_DIR/sys_root

pushd $OUTPUT_DIR/sys_root
gunzip -c $OUTPUT_DIR/initrd.gz | sudo cpio -ivd

cp -rf $SCRIPT_DIR/* .
if [ -d $BOARD_DIR/$TARGET ]; then
	cp -rf $BOARD_DIR/$TARGET/* .
fi

popd

sudo umount $OUTPUT_DIR/sys_root

rm -rf $OUTPUT_DIR/sys_root

gzip -f $OUTPUT_DIR/ramdisk

ls -al $OUTPUT_DIR/ramdisk.gz
