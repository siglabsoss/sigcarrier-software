Name:           artik-repos
Summary:        ARTIK Fedora package repositories
Version:        24
Release:        4
License:        MIT
Group:          System Environment/Base
URL:            http://repo.artik.cloud/artik/bin/pub/fedora/linux/releases/
Source:         %{name}-%{version}.tar.bz2
Provides:       artik-repos(%{version})
BuildArch:      noarch

%description
ARTIK Fedora package repository files for yum and dnf along with gpg public keys

%package artik530
Summary:	ARTIK530 Fedora package repository

%description artik530
ARTIK530 Fedora package repository files for yum and dnf along with gpg public keys

%package artik710
Summary:	ARTIK710 Fedora package repository

%description artik710
ARTIK710 Fedora package repository files for yum and dnf along with gpg public keys

%prep
%setup -q

%build

%install
# Install the keys
install -d -m 755 $RPM_BUILD_ROOT/etc/pki/rpm-gpg
install -m 644 fedora/RPM-GPG-KEY* $RPM_BUILD_ROOT/etc/pki/rpm-gpg/

install -d -m 755 $RPM_BUILD_ROOT/etc/yum.repos.d
for file in *.repo ; do
  install -m 644 fedora/$file $RPM_BUILD_ROOT/etc/yum.repos.d
done
find ./fedora/platform -name "*.repo" -exec install -m 644 {} $RPM_BUILD_ROOT/etc/yum.repos.d \;

%post artik530
sed -i '/^exclude=/s/$/,expat*/' /etc/yum.repos.d/archive-fedora-updates.repo

%files
%defattr(-,root,root,-)
%dir /etc/yum.repos.d
%config(noreplace) /etc/yum.repos.d/artik.repo
%dir /etc/pki/rpm-gpg
/etc/pki/rpm-gpg/*
%config(noreplace) /etc/yum.repos.d/archive-fedora*.repo

%files artik530
%defattr(-,root,root,-)
%config(noreplace) /etc/yum.repos.d/artik-platform-artik530*.repo

%files artik710
%defattr(-,root,root,-)
%config(noreplace) /etc/yum.repos.d/artik-platform-artik710*.repo
