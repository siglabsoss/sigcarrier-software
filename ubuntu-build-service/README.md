# Ubuntu Build Service

This repository provides to generate ubuntu rootfs by debian's live build system. It has been derived from linaro's [ubuntu-build-service](ubuntu-build-service).

## Directory structure
- xenial-arm64-artik710: ubuntu xenial(16.04) for artik710
- xenial-armhf-artik530: ubuntu xenial(16.04) for artik530

## Prerequisites
### Environment set up for ubuntu package build
#### Sbuild
The sbuild can be used to build a debian/ubuntu package and it will generate a deb package file. It generates chroot environment to make an isolated build environment. You can refer Ubuntu guide for [sbuild](https://wiki.ubuntu.com/SimpleSbuild)

- Install sbuild and schroot
```
sudo apt-get install build-essential devscripts quilt dh-autoreconf dh-systemd ubuntu-dev-tools sbuild debhelper moreutils
```

- Make sure you are in the 'sbuild' group:
```
sudo adduser $USER sbuild
```

- Setup for mounting $HOME and extracting the ddebs:
 + Create the directory for the mount point:
```
mkdir -p $HOME/ubuntu/scratch
```

 + Append to /etc/schroot/sbuild/fstab: You have to change <username> to yours.
```
/home/<username>/ubuntu/scratch  /scratch          none  rw,bind  0  0
```

- Edit ~/.sbuildrc and insert belows
```
$apt_allow_unauthenticated = 1;
$environment_filter = [
	'^PATH$',
	'^DEB(IAN|SIGN)?_[A-Z_]+$',
	'^(C(PP|XX)?|LD|F)FLAGS(_APPEND)?$',
	'^USER(NAME)?$',
	'^LOGNAME$',
	'^HOME$',
	'^TERM$',
	'^SHELL$',
	'^no_proxy$',
	'^http_proxy$',
	'^https_proxy$',
	'^ftp_proxy$',
];

# Directory for writing build logs to
$log_dir=$ENV{HOME}."/ubuntu/logs";

# don't remove this, Perl needs it:
1;
```

- Make the following directory (change if specified something different in ~/.sbuildrc):
```
mkdir -p $HOME/ubuntu/{build,logs}
```

- Create ~/.mk-sbuild.rc:
```
SCHROOT_CONF_SUFFIX="source-root-users=root,sbuild,admin
source-root-groups=root,sbuild,admin
preserve-environment=true"
# you will want to undo the below for stable releases, read `man mk-sbuild` for details
# during the development cycle, these pockets are not used, but will contain important
# updates after each release of Ubuntu
# if you have e.g. apt-cacher-ng around
# DEBOOTSTRAP_PROXY=http://127.0.0.1:3142/
```

- Enter the sbuild group if you were added during this session:
```
sg sbuild
```

- Generate a GPG keypair for sbuild to use: (You'll need to wait long time)
```
sudo sbuild-update --keygen
sudo chown -R $USER:sbuild ~/.gnupg/
```

- set up to copy /etc/hosts file into schroot
```
sudo su -c "grep -q /etc/hosts /etc/schroot/sbuild/copyfiles || echo /etc/hosts >> /etc/schroot/sbuild/copyfiles"
```

- Create a schroot environment for amd64-armhf(artik530) or amd64-arm64(artik710)
```
mk-sbuild --target armhf xenial
mk-sbuild --target arm64 xenial
```

- (armhf/artik530)Copy qemu-arm-static to schroot's /usr/bin
```
wget https://github.com/SamsungARTIK/fed-artik-tools/raw/artik/qemu-arm-static
chmod a+x qemu-arm-static
sudo cp qemu-arm-static /var/lib/schroot/chroots/xenial-amd64-armhf/usr/bin
```

- (arm64/artik710)Copy qemu-aarch64-static to schroot's /usr/bin
```
wget https://github.com/SamsungARTIK/fed-artik-tools/raw/artik/qemu-aarch64-static
chmod a+x qemu-aarch64-static
sudo cp qemu-aarch64-static /var/lib/schroot/chroots/xenial-amd64-arm64/usr/bin
```

## Environment set up for ubuntu rootfs build
### Live build
The live-build is a tool for debian live system. We can customize the debian/ubuntu distribution such as add/modify/delete a package. We can add a hook during build stage.

- Install live-build package from the source
```
sudo apt-get install qemu-user-static
cd ubuntu-build-service
cd packages
sudo dpkg -i live-build_3.0.5-1linaro1_all.deb
sudo apt-get install -f
```

## Ubuntu package build
To build a .deb package file from the source, you have to make debian/ directory and meta files for [debian build system](https://wiki.debian.org/Packaging/Intro?action=show&redirect=IntroDebianPackaging).

### Build with sbuild
- arm64(artik710)
```
cd bluez
sbuild --chroot xenial-amd64-arm64 --host arm64 --extra-repository="deb [trusted=yes] http://repo.artik.cloud/artik/bin/pub/artik-platform/ubuntu/artik710 A710_os_3.0.0 main" -j8
```
- armhf(artik530)
```
cd bluez
sbuild --chroot xenial-amd64-armhf --host armhf --extra-repository="deb [trusted=yes] http://repo.artik.cloud/artik/bin/pub/artik-platform/ubuntu/artik530 A530_os_3.0.0 main" -j8
```
- Build output
You can find the build results(deb, ddeb, src tarball) from the ../

## Ubuntu rootfs build
The rootfs of the ubuntu can be built using the live build(lb). You can generate a rootfs tarball from the xenial-{arch}-{artik_module} directory. You can see the documentation of the live build from [here](https://debian-live.alioth.debian.org/live-manual/unstable/manual/html/live-manual.en.html).

### Build with livebuild(lb)
- artik710
```
cd xenial-arm64-artik710
./configure
make
```

- artik530
```
cd xenial-armhf-artik530
./configure
make
```

- Build output
The rootfs tarball will be generated in each directory as "xenial-{artik_module}-YYMMDD-1.tar.gz". You can use the tarball for deploying artik image from build-artik.
```
cd build-artik
./release.sh -c config/artik710_ubuntu.cfg --local-rootfs ~/ubuntu-build-service/xenial-arm64-artik710/xenial-arm64-20170927-1.tar.gz
```

## Misc(Optional)
### sbuild with qemu
The ARTIK uses sbuild with cross build environment. It's faster than qemu build but some build can be not working due to the foreign package dependency. If you have the problem, you can change the cross build to qemu build.

#### sbuild(qemu arm64)
- Create a schroot environment for arm64
```
mk-sbuild --arch arm64 xenial
```

- build with xenial-arm64 chroot
```
sbuild --chroot xenial-arm64 --arch arm64 --extra-repository="deb [trusted=yes] http://repo.artik.cloud/artik/bin/pub/artik-platform/ubuntu/artik710 A710_os_3.0.0 main" -j8
```

#### sbuild(qemu arm)
- Create a schroot environment for arm
```
mk-sbuild --arch armhf xenial
```

- build with xenial-armhf chroot
```
sbuild --chroot xenial-armhf --arch armhf --extra-repository="deb [trusted=yes] http://repo.artik.cloud/artik/bin/pub/artik-platform/ubuntu/artik530 A530_os_3.0.0 main" -j8
```

### Set a local repository
You sometimes need newer packages (libraries, whatnot) to build against. Sometimes you might have a new dependency that isn't yet in the archive, but you are working on the new packaging.

- Clone repotools to your scratch:
```
git clone https://git.launchpad.net/~barry/+git/repotools ~/ubuntu/repo
```

- Install sponge
```
sudo apt install moreutils
```

- Modify clean.sh and scan.sh to set the repository path:
```
cd ~/ubuntu/repo
sed -i "s|/home/barry|/home/$USER|g" *
```

- Make update_repo.sh on your scratch directory because all build output will be located top of the build directory(~/ubuntu/scratch directory). This script will move all build output on your ~/ubuntu/debs.
```
cat > ~/ubuntu/scratch/update_repo.sh << __EOF__
#!/bin/bash

set -x

SCRIPT_DIR=`dirname "$(readlink -f "$0")"`

pushd \$SCRIPT_DIR
mv *.changes *.dsc *.deb *.ddeb *.tar.* *.udeb *.build ~/ubuntu/debs
~/ubuntu/repo/scan.sh
popd
__EOF__

chmod a+x ~/ubuntu/scratch/update_repo.sh
```

- Add the following snippet to your ~/.sbuildrc:
```
$external_commands = {
	"pre-build-commands" => [
		['/home/'.$ENV{USER}.'/ubuntu/repo/scan.sh'],
	],
	"chroot-setup-commands" => [
		['/repo/prep.sh'],
	],
	"post-build-commands" => [
		['/home/'.$ENV{USER}.'/ubuntu/scratch/update_repo.sh'],
	],
};
```

- And add the following lines to /etc/schroot/sbuild/fstab: You have to change <username> to yours.
```
# Expose local apt repository to the chroot
/home/<username>/ubuntu/repo     /repo             none  ro,bind  0  0
```

- Create a directory to hold your local debs:
```
mkdir ~/ubuntu/debs
```

- Now, post build command will move the build output into the ~/ubuntu/debs directory

- Start an http server that will act as a local repository:
```
cd ~/ubuntu/debs
python3 -m http.server 8000 --bind 127.0.0.1
```

- If you want to refer the local repository during build, you can do that by editing ~/.sbuildrc.
```
$extra_repositories = ['deb [trusted=yes] http://localhost:8000 ./', 'deb-src [trusted=yes] http://localhost:8000 ./' ];
```

### Append a package for the rootfs
- If you want to add your own package, you have to build it with sbuild and put the deb into the local repository.
```
cp *.deb ~/ubuntu/debs
~/ubuntu/scan.sh
cd ~/ubuntu/debs
python3 -m http.server 8000 --bind 127.0.0.1
```
- artik710) Append a package name of the "xenial-arm64-artik710/customization/package-lists/package.list.chroot"
- artik530) Append a package name of the "xenial-armhf-artik530/customization/package-lists/package.list.chroot"
- Build
```
cd xenial-arm64-artik710
./configure
make
```

