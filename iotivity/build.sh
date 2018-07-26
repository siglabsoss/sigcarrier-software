#!/bin/sh -e

if [ $# -ne 3 ]; then
	echo "Usage: $0 {spec-file} {source-dir} {sysroot-dir}"
	exit 1
fi

filename=$1
targetpath=$2
sysrootpath=$3

if [ ! -f $filename ]; then
	echo "$filename not found."
	exit 1
fi

if [ ! -d $targetpath ]; then
	echo "$targetpath not found."
	exit 1
fi

mkdir -p ~/rpmbuild/SOURCES
mkdir -p ~/rpmbuild/SPECS

version=`rpmspec --query --srpm --queryformat="%{version}" ${filename}`
name=`rpmspec --query --srpm --queryformat="%{name}" ${filename}`
buildname=${name}-${version}
src=${buildname}.tar.gz

# tar.gz SHOULD contain ${buildname} prefix path
tmppath=/tmp/${buildname}
rm -rf $tmppath
mkdir -p $tmppath
cp -a ${targetpath}/* $tmppath/
tar cvfz ~/rpmbuild/SOURCES/$src -C /tmp ${buildname} >/dev/null
rm -rf $tmppath

# build rpm
rpmbuild -ba $filename --nodeps --define "_sysrootdir ${sysrootpath}" --target=armv7hl
