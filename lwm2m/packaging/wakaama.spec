Name: wakaama
Version: 1.0
Release: 1%{?dist}
License: EDL&EPL
Summary: Implementation of the Open Mobile Alliance's LightWeight M2M protocol (LWM2M)
Group: Development/Libraries
URL: https://github.com/obgm/wakaama
Source: %{name}-%{version}.tar.gz

BuildRequires: cmake

%description
Implementation of the Open Mobile Alliance's LightWeight M2M protocol

%package devel
Summary: Development files for implementation of the Open Mobile Alliance's LightWeight M2M protocol (LWM2M)
Group: Development/Libraries
License: EDL&EPL
Requires: %{name} = %{version}-%{release}

%description devel
Implementation of the Open Mobile Alliance's LightWeight M2M protocol (devel)

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
%doc EDL-v1.0 EPL-v1.0 README
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/*
