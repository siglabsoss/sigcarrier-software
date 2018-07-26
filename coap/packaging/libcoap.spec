Name: libcoap
Version: 4.1.1
Release: 1%{?dist}
License: Dual BSD/GPLv2
Summary: C-Implementation of CoAP
Group: Development/Libraries
URL: https://github.com/obgm/libcoap
Source: %{name}-%{version}.tar.gz
#Patch0: libcoap_build.patch

BuildRequires: autoconf
BuildRequires: automake

%description
C-Implementation of CoAP

%package devel
Summary: Development files for the C-Implementation of CoAP
Group: Development/Libraries
License: BSD+GPL
Requires: %{name} = %{version}-%{release}

%description devel
C-Implementation of CoAP (devel)

%prep
%setup -q
#%patch0 -p1

%build
autoreconf --force --verbose --install
%configure --with-shared
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc AUTHORS README LICENSE.BSD LICENSE.GPL
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_libdir}/*.a
%{_includedir}/*
