Name: TomyGateway
Summary: MQTT-SN gateway application
Version: 1.1.1
Release: 1%{?dist}
Group: Applications/Communications
License: BSD
Source: %{name}-%{version}.tar.gz
BuildRequires: openssl-devel

%description
MQTT-SN gateway application

%prep
%setup -q

%build
cd Gateway
make clean
make
cd -

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_prefix}/local/etc/tomygateway/config
cp Gateway/Build/TomyGateway %{buildroot}%{_bindir}
cp Gateway/param.conf %{buildroot}%{_prefix}/local/etc/tomygateway/config/

%files
%attr(0555,root,root) %{_bindir}/TomyGateway
%attr(0666,root,root) %{_prefix}/local/etc/tomygateway/config/param.conf
