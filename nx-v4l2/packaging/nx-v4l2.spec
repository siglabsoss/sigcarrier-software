Name:    nx-v4l2
Version: 1.0.3
Release: 1
License: LGPLv2+
Summary: Nexell v4l2 library
Group: Development/Libraries
Source:  %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig automake autoconf libtool
BuildRequires:  pkgconfig(glib-2.0)

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Nexell v4l2 library

%package devel
Summary: Nexell v4l2 library
Group: Development/Libraries
License: LGPLv2+
Requires: %{name} = %{version}-%{release}

%description devel
Nexell v4l2 library (devel)

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
cp %{_builddir}/%{name}-%{version}/media-bus-format.h %{buildroot}/usr/include
cp %{_builddir}/%{name}-%{version}/nx-v4l2.h %{buildroot}/usr/include

%files
%{_libdir}/libnx_v4l2.so
%{_libdir}/libnx_v4l2.so.*
%license LICENSE.LGPLv2+

%files devel
%{_includedir}/media-bus-format.h
%{_includedir}/nx-v4l2.h
%{_includedir}/mm_types.h
%license LICENSE.LGPLv2+
