Name: mqttsn
Version: 1.0
Release: 1%{?dist}
License: EDL&EPL
Summary: Eclipse Paho MQTT-SN Embedded C client
Group: Development/Libraries
URL: https://github.com/eclipse/paho.mqtt-sn.embedded-c
Source: %{name}-%{version}.tar.gz
BuildRequires: cmake

%description
Eclipse Paho MQTT-SN Embedded C client

%package devel
Summary: Development files for Eclipse Paho MQTT-SN Embedded C client
Group: Development/Libraries
License: EDL&EPL
Requires: %{name} = %{version}-%{release}

%description devel
Eclipse Paho MQTT-SN Embedded C client (devel)

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

%files
%defattr(-,root,root,-)
%doc edl-v10 epl-v10 about.html CONTRIBUTING.md notice.html
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/*
