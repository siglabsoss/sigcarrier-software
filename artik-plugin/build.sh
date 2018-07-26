#!/bin/sh

if [ -n "$1" ]
then
echo "build $1"
else
echo "build.sh [TARGET]"
exit
fi

[ -d ~/rpmbuild/SOURCES ] || mkdir -p ~/rpmbuild/SOURCES
[ -d ~/rpmbuild/RPMS ] || mkdir -p ~/rpmbuild/RPMS
[ -d ~/rpmbuild/SRPMS ] || mkdir -p ~/rpmbuild/SRPMS

name=artik-plugin
version=`rpmspec --query --srpm --queryformat="%{version}" packaging/${name}.spec`
buildname=${name}-${version}

git archive --format=tar.gz --prefix=$buildname/ -o ~/rpmbuild/SOURCES/$buildname.tar.gz HEAD

rpmbuild --target=armv7hl -ba --define "TARGET $1" ./packaging/artik-plugin.spec
