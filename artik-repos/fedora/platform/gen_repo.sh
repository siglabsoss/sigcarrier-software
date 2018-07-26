#!/bin/bash

TARGET=$1
VERSION=$2

usage() {
	echo "./gen_repo.sh TARGET VERSION"
	exit 0
}

if [ "$TARGET" == "-h" ]; then
	usage
fi

if [ "$VERSION" == "" ]; then
	usage
fi

[ -d $TARGET ] || mkdir $TARGET
cat artik-platform.repo.in | sed -e "s/_TARGET_NAME_/${TARGET}/g; s/_RELEASE_VERSION_/${VERSION}/g" > $TARGET/artik-platform-${TARGET}.repo
cat artik-platform-updates.repo.in | sed -e "s/_TARGET_NAME_/${TARGET}/g; s/_RELEASE_VERSION_/${VERSION}/g" > $TARGET/artik-platform-${TARGET}-updates.repo
