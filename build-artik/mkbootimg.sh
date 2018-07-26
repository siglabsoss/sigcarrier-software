#!/bin/bash

set -e

print_usage()
{
	echo "-h/--help         Show help options"
	echo "-b [TARGET_BOARD]	Target board ex) -b artik710|artik530|artik5|artik10"

	exit 0
}

parse_options()
{
	for opt in "$@"
	do
		case "$opt" in
			-h|--help)
				print_usage
				shift ;;
			-b)
				TARGET_BOARD="$2"
				shift ;;
		esac
	done
}

die() {
	if [ -n "$1" ]; then echo $1; fi
	exit 1
}

gen_boot_image()
{
	dd if=/dev/zero of=boot.img bs=1M count=$BOOT_SIZE
	if [ "$BOOT_PART_TYPE" == "vfat" ]; then
		sudo sh -c "mkfs.vfat -n boot boot.img"
	else
		if [ "$BOOT_PART_TYPE" == "ext4" ]; then
			sudo sh -c "mkfs.ext4 -F -L boot -b 4096 boot.img"
		fi
	fi
}

install_boot_image()
{
	test -d mnt || mkdir mnt
	sudo mount -o loop boot.img mnt

	sudo sh -c "install -m 664 $TARGET_DIR/$KERNEL_IMAGE mnt"
	sudo sh -c "install -m 664 $TARGET_DIR/$KERNEL_DTB mnt"
	sudo sh -c "install -m 664 $TARGET_DIR/$RAMDISK_NAME mnt"

	sync; sync;
	sudo umount mnt

	rm -rf mnt
}

trap 'error ${LINENO} ${?}' ERR
parse_options "$@"

SCRIPT_DIR=`dirname "$(readlink -f "$0")"`

if [ "$TARGET_BOARD" == "" ]; then
	print_usage
else
	if [ "$TARGET_DIR" == "" ]; then
		. $SCRIPT_DIR/config/$TARGET_BOARD.cfg
	fi
fi

test -e $TARGET_DIR/$KERNEL_IMAGE || die "not found"
test -e $INITRD || die "not found"

cp $INITRD $TARGET_DIR/$RAMDISK_NAME

pushd $TARGET_DIR

gen_boot_image
install_boot_image

popd
