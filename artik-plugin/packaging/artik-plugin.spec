Name:		artik-plugin
Summary:	ARTIK plugin files for fedora
Version:	1.0.12
Release:	1
Group:		System Environment/Base
License:	none

Requires:	systemd
Requires:	setup
Requires:	dnsmasq
Requires:	java-1.8.0-openjdk
Requires:	fedora-repos
Source0:	%{name}-%{version}.tar.gz

%description
ARTIK plugin files for fedora

%prep
%setup -q

%install
rm -rf %{buildroot}

# determine arch and OS for rpm
mkdir -p %{buildroot}/etc/rpm
cp -f scripts/platform %{buildroot}/etc/rpm
%ifarch aarch64
echo "aarch64-fedora-linux-gnu" > %{buildroot}/etc/rpm/platform
%endif

mkdir -p  %{buildroot}/etc/bluetooth
cp -r prebuilt/bluetooth/common/* %{buildroot}/etc/bluetooth

mkdir -p %{buildroot}/usr/lib/systemd/system
cp scripts/units/rfkill-unblock.service %{buildroot}/usr/lib/systemd/system

mkdir -p %{buildroot}/etc/udev/rules.d
cp scripts/rules/10-local.rules %{buildroot}/etc/udev/rules.d

mkdir -p %{buildroot}/etc/profile.d
cp scripts/open-jdk.sh %{buildroot}/etc/profile.d

# network
mkdir -p %{buildroot}/etc/sysconfig/network-scripts
cp prebuilt/network/ifcfg-eth0 %{buildroot}/etc/sysconfig/network-scripts

mkdir -p %{buildroot}/etc/modules-load.d
cp scripts/modules-load.d/tun.conf %{buildroot}/etc/modules-load.d/

cp -r prebuilt/connman/* %{buildroot}

# audio
mkdir -p %{buildroot}/usr/lib/systemd/system
cp scripts/units/pulseaudio.service %{buildroot}/usr/lib/systemd/system
cp scripts/units/audiosetting.service %{buildroot}/usr/lib/systemd/system

# adbd
mkdir -p %{buildroot}/usr/bin
cp prebuilt/adbd/adbd %{buildroot}/usr/bin

mkdir -p %{buildroot}/usr/lib/systemd/system
cp scripts/units/adbd.service %{buildroot}/usr/lib/systemd/system
cp scripts/units/rndis.service %{buildroot}/usr/lib/systemd/system
cp scripts/rules/99-adb-restart.rules %{buildroot}/etc/udev/rules.d

# systemd module load service
mkdir -p %{buildroot}/etc/systemd/system
cp scripts/units/systemd-modules-load.service %{buildroot}/etc/systemd/system

# booting done service
mkdir -p %{buildroot}/usr/bin
cp scripts/booting-done.sh %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/lib/systemd/system
cp scripts/units/booting-done.service %{buildroot}/usr/lib/systemd/system

# umount /usr/lib/modules
mkdir -p %{buildroot}/usr/lib/systemd/system
cp scripts/units/modules-umount.service %{buildroot}/usr/lib/systemd/system

# wifi
mkdir -p %{buildroot}/usr/bin
cp prebuilt/wifi/SimpleWiFi.py %{buildroot}/usr/bin

# logdump
mkdir -p %{buildroot}/usr/bin
cp scripts/logdump.sh %{buildroot}/usr/bin

# licenses
mkdir -p %{buildroot}/usr/share
cp -r licenses %{buildroot}/usr/share

# archive repo
mkdir -p %{buildroot}/etc/yum.repos.d
cp -f scripts/repos/* %{buildroot}/etc/yum.repos.d

%post
# Setting default runlevel to multi-user text mode
rm -f /etc/systemd/system/default.target
ln -s /lib/systemd/system/multi-user.target /etc/systemd/system/default.target

# Limit journal size
sed -i "s/#SystemMaxUse=/SystemMaxUse=10M/" /etc/systemd/journald.conf

# create directory for journal log on disk
mkdir -p /var/log/journal

# reset hardware watchdog
sed -i 's/#ShutdownWatchdogSec=10min/ShutdownWatchdogSec=10s/g' /etc/systemd/system.conf

# wpa_supplicant
sed -i 's/INTERFACES=\"\"/INTERFACES=\"-iwlan0\"/g' /etc/sysconfig/wpa_supplicant
sed -i 's/DRIVERS=\"\"/DRIVERS=\"-Dnl80211\"/g' /etc/sysconfig/wpa_supplicant

# ignore power key action
sed -i "s/#HandlePowerKey=poweroff/HandlePowerKey=ignore/" /etc/systemd/logind.conf

# Set tcp pacing ca ratio to 200
if [ `grep -c tcp_pacing_ca_ratio /etc/sysctl.conf` != 0 ]; then
	sed -i "s/net.ipv4.tcp_pacing_ca_ratio=.*/net.ipv4.tcp_pacing_ca_ratio=200/g" /etc/sysctl.conf
else
	echo "net.ipv4.tcp_pacing_ca_ratio=200" >> /etc/sysctl.conf
fi

# Enable units
systemctl enable systemd-timesyncd.service
systemctl enable systemd-resolved.service
systemctl enable booting-done.service
systemctl enable rfkill-unblock.service
systemctl enable modules-umount.service

# systemd module load service
systemctl enable systemd-modules-load.service

# Dnsmasq setting
sed -i 's/\#except-interface=/except-interface=lo/g'  /etc/dnsmasq.conf

# Install java alternatives
/usr/sbin/alternatives --install /usr/bin/java java /usr/java/default/jre/bin/java 1
/usr/sbin/alternatives --install /usr/bin/javaws javaws /usr/java/default/jre/bin/javaws 1
/usr/sbin/alternatives --install /usr/bin/javac javac /usr/java/default/bin/javac 1
/usr/sbin/alternatives --install /usr/bin/jar jar /usr/java/default/bin/jar 1

# Sync after sshd key generation
sed -i 's/\[Service\]/\[Service\]\nExecStartPost=\/usr\/bin\/sync/g' /usr/lib/systemd/system/sshd-keygen\@.service

###############################################################################
# artik-plugin

%files
%attr(0644,root,root) /etc/rpm/platform
%attr(0644,root,root) /etc/systemd/system/systemd-modules-load.service
%attr(0755,root,root) /usr/bin/booting-done.sh
%attr(0644,root,root) /usr/lib/systemd/system/booting-done.service
%attr(0644,root,root) /etc/profile.d/open-jdk.sh
%attr(0644,root,root) /usr/lib/systemd/system/rfkill-unblock.service
%attr(0644,root,root) /usr/lib/systemd/system/modules-umount.service

# logdump
%attr(0755,root,root) /usr/bin/logdump.sh

# archive repos
%attr(0644,root,root) /etc/yum.repos.d/*.repo

###############################################################################
# Bluetooth
# ARTIK common
%package bluetooth-common
Summary:	bluetooth
Group:		System
Requires:	bluez

%description bluetooth-common
Bluetooth

%post bluetooth-common
systemctl enable bluetooth.service

%files bluetooth-common
%attr(0644,root,root) /etc/udev/rules.d/10-local.rules
/etc/bluetooth/*

###############################################################################
# network
# ARTIK common
%package network-common
Summary:	network
Group:		System
Requires:	connman

%description network-common
Network Driver and DHCP configuration

%post network-common
systemctl enable connman.service

%files network-common
%attr(0644,root,root) /etc/sysconfig/network-scripts/ifcfg-eth0
%attr(0644,root,root) /etc/connman/main.conf
%attr(0644,root,root) /var/lib/connman/settings
%attr(0644,root,root) /etc/modules-load.d/tun.conf

###############################################################################
# audio
# ARTIK common
%package audio-common
Summary:	audio
Group:		System
Requires:	pulseaudio

%description audio-common
audio

%post audio-common
systemctl enable pulseaudio.service
systemctl enable audiosetting.service

sed -i 's/; exit-idle-time = 20/exit-idle-time = -1/g' /etc/pulse/daemon.conf
sed -i 's/load-module module-udev-detect/load-module module-udev-detect tsched=0/g' /etc/pulse/default.pa
echo "load-module module-switch-on-connect" >> /etc/pulse/default.pa
cp /etc/pulse/default.pa /etc/pulse/system.pa

/usr/sbin/usermod -G pulse-access root
/usr/sbin/usermod -a -G audio pulse

# pulseaudio settings for bluetooth a2dp_sink
sed -i '/<allow own="org.pulseaudio.Server"\/>/a \ \ \ \ <allow send_destination="org.bluez"/>' /etc/dbus-1/system.d/pulseaudio-system.conf

%files audio-common
%attr(0644,root,root) /usr/lib/systemd/system/pulseaudio.service
%attr(0644,root,root) /usr/lib/systemd/system/audiosetting.service

###############################################################################
# usb gadget
%package usb-common
Summary:	usb
Group:		System

%description usb-common
usb

%files usb-common
%attr(0755,root,root) /usr/bin/adbd
%attr(0644,root,root) /usr/lib/systemd/system/adbd.service
%attr(0644,root,root) /usr/lib/systemd/system/rndis.service
%attr(0644,root,root) /etc/udev/rules.d/99-adb-restart.rules

###############################################################################
# wifi
%package wifi-common
Summary:	wifi
Group:		System
Requires:	python3

%description wifi-common
wifi

%files wifi-common
%attr(0755,root,root) /usr/bin/SimpleWiFi.py

###############################################################################
# license
%package license
Summary:	license
Group:		System

%description license
license

%files license
%attr(0644,root,root) /usr/share/licenses/*
