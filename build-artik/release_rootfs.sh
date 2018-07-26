#!/bin/bash

set -x
set -e

CHECK_COUNT=0
MAX_RETRY=3

test -d ${TARGET_DIR} || mkdir -p ${TARGET_DIR}

urlencode() {
    # urlencode <string>
    local length="${#1}"
    for (( i = 0; i < length; i++ )); do
        local c="${1:i:1}"
        case $c in
            [a-zA-Z0-9.~_-]) printf "$c" ;;
            *) printf '%%%02X' "'$c"
        esac
    done
}

ROOTFS_TAG=$(urlencode "$ROOTFS_TAG")
ROOTFS_FILE=$(urlencode "$ROOTFS_FILE")

if [ ! -f $PREBUILT_DIR/$ROOTFS_FILE ]; then
	echo "Not found rootfs. Download it"
	wget ${ROOTFS_BASE_URL}/${ROOTFS_TAG}/${ROOTFS_FILE} -O $PREBUILT_DIR/$ROOTFS_FILE
fi

while :
do
	MD5_SUM=$(md5sum $PREBUILT_DIR/$ROOTFS_FILE | awk '{print $1}')
	if [ "$ROOTFS_FILE_MD5" == "$MD5_SUM" ]; then
		break
	fi

	echo "Mismatch MD5 hash. Just download again"
	wget ${ROOTFS_BASE_URL}/${ROOTFS_TAG}/${ROOTFS_FILE} -O $PREBUILT_DIR/$ROOTFS_FILE

	CHECK_COUNT=$((CHECK_COUNT + 1))

	if [ $CHECK_COUNT -ge $MAX_RETRY ]; then
		exit -1
	fi
done

cp $PREBUILT_DIR/$ROOTFS_FILE $TARGET_DIR/rootfs.tar.gz
