Name:    nx-renderer
Version: 1.0.0
Release: 1
License: LGPLv2+
Summary: Nexell drm renderer library
Group: Development/Libraries
Source:  %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig automake autoconf libtool
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  libdrm-devel

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Nexell renderer library

%package devel
Summary: Nexell drm renderer library
Group: Development/Libraries
License: LGPLv2+
Requires: %{name} = %{version}-%{release}

%description devel
Nexell drm renderer library (devel)

%prep
%setup -q

%build
autoreconf -v --install || exit 1
%configure
make %{?_smp_mflags}

%postun -p /sbin/ldconfig

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

find %{buildroot} -type f -name "*.la" -delete

mkdir -p %{buildroot}/usr/include
cp %{_builddir}/%{name}-%{version}/include/dp.h %{buildroot}/usr/include
cp %{_builddir}/%{name}-%{version}/include/dp_common.h %{buildroot}/usr/include

%files
%{_libdir}/libnx_renderer.so
%{_libdir}/libnx_renderer.so.*
%license LICENSE.LGPLv2+

%files devel
%{_includedir}/dp.h
%{_includedir}/dp_common.h
%license LICENSE.LGPLv2+
