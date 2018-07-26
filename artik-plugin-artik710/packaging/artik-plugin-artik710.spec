%define __jar_repack 0

Name:		artik-plugin-artik710
Summary:	ARTIK710 plugin files for fedora
Version:	1.0.4
Release:	1
Group:		System Environment/Base
License:	none

Source0:	%{name}-%{version}.tar.gz

%description
ARTIK710 plugin files for fedora

%prep
%setup -q

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}/etc/modules-load.d
cp scripts/modules-load.d/* %{buildroot}/etc/modules-load.d

mkdir -p %{buildroot}/etc/modprobe.d
cp scripts/modprobe.d/dhd.conf %{buildroot}/etc/modprobe.d/

mkdir -p  %{buildroot}/etc/bluetooth
cp -r prebuilt/bluetooth/etc/bluetooth/* %{buildroot}/etc/bluetooth

mkdir -p %{buildroot}/usr/lib/systemd/system
cp scripts/units/brcm-firmware.service %{buildroot}/usr/lib/systemd/system

# fstab
mkdir -p %{buildroot}/etc
cp prebuilt/fstab/fstab %{buildroot}/etc/

# audio
mkdir -p %{buildroot}/usr/bin
cp -r prebuilt/audio/audio_setting.sh %{buildroot}/usr/bin

# wifi
mkdir -p %{buildroot}/etc/wifi
cp -r prebuilt/wifi/* %{buildroot}/etc/wifi

# adbd
mkdir -p %{buildroot}/usr/bin
cp -r prebuilt/adbd/* %{buildroot}/usr/bin
cp -r prebuilt/rndis/* %{buildroot}/usr/bin

# zigbee
mkdir -p %{buildroot}/etc/zigbee
cp prebuilt/zigbee/tx_power %{buildroot}/etc/zigbee/tx_power

# security
mkdir -p %{buildroot}/usr/bin
cp -r prebuilt/security/fwupd %{buildroot}/usr/bin

# licenses
mkdir -p %{buildroot}/usr/share
cp -r licenses %{buildroot}/usr/share

# network priority
mkdir -p %{buildroot}/usr/lib/systemd/system
cp scripts/units/set-network-priority.service %{buildroot}/usr/lib/systemd/system

%package bluetooth
Summary:    bluetooth
Group:		System
Requires:	bluez
Requires:	artik-plugin-bluetooth-common

%description bluetooth
Bluetooth

%post bluetooth
systemctl enable brcm-firmware.service

%files bluetooth
%attr(0644,root,root) /usr/lib/systemd/system/brcm-firmware.service
/etc/bluetooth/*

%package fstab
Summary:    fstab
Group:		System
Requires:	artik-plugin

%description fstab
fstab

%files fstab
%attr(0644,root,root) /etc/fstab

%package audio
Summary:    audio
Group:		System
Requires:       pulseaudio
Requires:	artik-plugin-audio-common

%description audio
audio

%files audio
%attr(0755,root,root) /usr/bin/audio_setting.sh

%package wifi
Summary:    wifi
Group:		System
Requires:	connman

%description wifi
wifi

%post wifi
systemctl enable set-network-priority.service

%files wifi
%attr(0644,root,root) /etc/modules-load.d/dhd.conf
%attr(0644,root,root) /etc/modprobe.d/dhd.conf
/etc/wifi/*
%attr(0644,root,root) /usr/lib/systemd/system/set-network-priority.service

%package usb
Summary:    artik710 usb package
Group:		System
Requires:	artik-plugin-usb-common

%description usb
artik710 usb package

%files usb
%attr(0755,root,root) /usr/bin/start_adbd.sh
%attr(0755,root,root) /usr/bin/start_rndis.sh

%package zigbee
Summary:    zigbee
Group:		System

%description zigbee
zigbee

%files zigbee
%attr(0644,root,root) /etc/zigbee/tx_power

%package security
Summary:    security
Group:		System

%description security
security

%post security

%files security
%attr(0755,root,root) /usr/bin/fwupd

%package license
Summary:	license
Group:		System

%description license
license

%files license
%attr(0644,root,root) /usr/share/licenses/*
