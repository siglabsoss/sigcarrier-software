%define native %(echo %arm | egrep -c %{_host_cpu})

Name: iotivity
Version: 1.1.0
Release: 0%{?dist}
Summary: IoT Connectivity sponsored by the OIC
Group: Network & Connectivity/Other
License: Apache-2.0
URL: https://www.iotivity.org/
Source: %{name}-%{version}.tar.gz
BuildRequires:  gettext, expat-devel
BuildRequires:  python, libcurl-devel
BuildRequires:  scons
BuildRequires:  openssl-devel
BuildRequires:  boost-devel
BuildRequires:  boost-thread
BuildRequires:  boost-system
BuildRequires:  boost-filesystem
BuildRequires:  pkgconfig(uuid)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  chrpath
Requires(postun): /sbin/ldconfig
Requires(post): /sbin/ldconfig


#%define RELEASE False
%define RELEASE True

%{!?TARGET_TRANSPORT: %global TARGET_TRANSPORT IP}
%{!?SECURED: %global SECURED 0}
%{!?LOGGING: %global LOGGING True}
%{!?ROUTING: %global ROUTING GW}
%{!?ES_TARGET_ENROLLEE: %global ES_TARGET_ENROLLEE linux}
%{!?ES_ROLE: %global ES_ROLE enrollee}
%{!?ES_SOFTAP_MODE: %global ES_SOFTAP_MODE MEDIATOR_SOFTAP}

%description
An open source reference implementation of the OIC standard specifications
IoTivity Base Libraries are included.


%package service
Summary: Development files for %{name}
Group: Network & Connectivity/Service
Requires: %{name} = %{version}-%{release}

%description service
The %{name}-service package contains service libraries files for
developing applications that use %{name}-service.

%package test
Summary: Development files for %{name}
Group: Network & Connectivity/Testing
Requires: %{name} = %{version}-%{release}

%description test
The %{name}-test package contains example files to show
how the iotivity works using %{name}-test

%package devel
Summary: Development files for %{name}
Group: Network & Connectivity/Development
Requires: %{name} = %{version}-%{release}
Requires: pkgconfig

%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%prep
%setup -q
chmod g-w %_sourcedir/*

cp LICENSE.md LICENSE.APLv2

%build
%define RPM_ARCH %{_arch}

%ifarch armv7l armv7hl armv7nhl armv7tnhl armv7thl
%define RPM_ARCH "armeabi-v7a-hard"
%endif

%ifarch aarch64
%define RPM_ARCH "arm64"
%endif

%ifarch x86_64
%define RPM_ARCH "x86_64"
%endif

%ifarch %{ix86}
%define RPM_ARCH "x86"
%endif

%if %{native}
scons %{?_smp_mflags} --prefix=%{_prefix} \
	TARGET_OS=linux TARGET_ARCH=%{RPM_ARCH} \
	TARGET_TRANSPORT=%{TARGET_TRANSPORT} \
	RELEASE=%{RELEASE} SECURED=%{SECURED} LOGGING=0 \
	ES_TARGET_ENROLLEE=%{ES_TARGET_ENROLLEE} ES_ROLE=%{ES_ROLE} \
	VERBOSE=yes LIB_INSTALL_DIR=%{_libdir}
%else
export PKG_CONFIG_SYSROOT_DIR=%{_sysrootdir}
export PKG_CONFIG_PATH=${PKG_CONFIG_SYSROOT_DIR}/usr/lib/pkgconfig
VERBOSE=1
scons %{?_smp_mflags} --prefix=%{_prefix} \
	TARGET_OS=linux TARGET_ARCH=%{RPM_ARCH} \
	TARGET_TRANSPORT=%{TARGET_TRANSPORT} \
	RELEASE=%{RELEASE} SECURED=%{SECURED} LOGGING=0 \
	ES_TARGET_ENROLLEE=%{ES_TARGET_ENROLLEE} ES_ROLE=%{ES_ROLE} \
	VERBOSE=yes TC_PREFIX=arm-linux-gnueabihf- TC_PATH=%{_sysrootdir}/usr \
	LIB_INSTALL_DIR=%{_libdir}
%endif

%install
rm -rf %{buildroot}
CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS ;

%if %{native}
scons install --install-sandbox=%{buildroot} --prefix=%{_prefix} \
	TARGET_OS=linux TARGET_ARCH=%{RPM_ARCH} \
	TARGET_TRANSPORT=%{TARGET_TRANSPORT} \
	RELEASE=%{RELEASE} SECURED=%{SECURED} LOGGING=0 \
	ES_TARGET_ENROLLEE=%{ES_TARGET_ENROLLEE} ES_ROLE=%{ES_ROLE} \
	VERBOSE=yes LIB_INSTALL_DIR=%{_libdir}
%else
export PKG_CONFIG_SYSROOT_DIR=%{_sysrootdir}
export PKG_CONFIG_PATH=${PKG_CONFIG_SYSROOT_DIR}/usr/lib/pkgconfig
scons install --install-sandbox=%{buildroot} --prefix=%{_prefix} \
	TARGET_OS=linux TARGET_ARCH=%{RPM_ARCH} \
	TARGET_TRANSPORT=%{TARGET_TRANSPORT} \
	RELEASE=%{RELEASE} SECURED=%{SECURED} LOGGING=0 \
	ES_TARGET_ENROLLEE=%{ES_TARGET_ENROLLEE} ES_ROLE=%{ES_ROLE} \
	VERBOSE=yes TC_PREFIX=arm-linux-gnueabihf- TC_PATH=%{_sysrootdir}/usr \
	LIB_INSTALL_DIR=%{_libdir}
%endif

# For Example
%if %{RELEASE} == "True"
%define build_mode release
%else
%define build_mode debug
%endif
%define ex_install_dir %{buildroot}%{_bindir}
mkdir -p %{ex_install_dir}

cp out/linux/*/%{build_mode}/lib*.so %{buildroot}%{_libdir}
cp out/linux/*/%{build_mode}/lib*.a %{buildroot}%{_libdir}

cp out/linux/*/%{build_mode}/examples/OICMiddle/OICMiddle %{ex_install_dir}
chrpath -d %{ex_install_dir}/OICMiddle
cp out/linux/*/%{build_mode}/resource/examples/devicediscoveryclient %{ex_install_dir}
chrpath -d %{ex_install_dir}/devicediscoveryclient
cp out/linux/*/%{build_mode}/resource/examples/devicediscoveryserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/devicediscoveryserver
cp out/linux/*/%{build_mode}/resource/examples/fridgeclient %{ex_install_dir}
chrpath -d %{ex_install_dir}/fridgeclient
cp out/linux/*/%{build_mode}/resource/examples/fridgeserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/fridgeserver
cp out/linux/*/%{build_mode}/resource/examples/garageclient %{ex_install_dir}
chrpath -d %{ex_install_dir}/garageclient
cp out/linux/*/%{build_mode}/resource/examples/garageserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/garageserver
cp out/linux/*/%{build_mode}/resource/examples/groupclient %{ex_install_dir}
chrpath -d %{ex_install_dir}/groupclient
cp out/linux/*/%{build_mode}/resource/examples/groupserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/groupserver
cp out/linux/*/%{build_mode}/resource/examples/lightserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/lightserver
cp out/linux/*/%{build_mode}/resource/examples/presenceclient %{ex_install_dir}
chrpath -d %{ex_install_dir}/presenceclient
cp out/linux/*/%{build_mode}/resource/examples/presenceserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/presenceserver
cp out/linux/*/%{build_mode}/resource/examples/roomclient %{ex_install_dir}
chrpath -d %{ex_install_dir}/roomclient
cp out/linux/*/%{build_mode}/resource/examples/roomserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/roomserver
cp out/linux/*/%{build_mode}/resource/examples/simpleclient %{ex_install_dir}
chrpath -d %{ex_install_dir}/simpleclient
cp out/linux/*/%{build_mode}/resource/examples/simpleclientHQ %{ex_install_dir}
chrpath -d %{ex_install_dir}/simpleclientHQ
cp out/linux/*/%{build_mode}/resource/examples/simpleclientserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/simpleclientserver
cp out/linux/*/%{build_mode}/resource/examples/simpleserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/simpleserver
cp out/linux/*/%{build_mode}/resource/examples/simpleserverHQ %{ex_install_dir}
chrpath -d %{ex_install_dir}/simpleserverHQ
cp out/linux/*/%{build_mode}/resource/examples/threadingsample %{ex_install_dir}
chrpath -d %{ex_install_dir}/threadingsample
cp out/linux/*/%{build_mode}/resource/csdk/stack/samples/linux/SimpleClientServer/occlient %{ex_install_dir}
chrpath -d %{ex_install_dir}/occlient
cp out/linux/*/%{build_mode}/resource/csdk/stack/samples/linux/SimpleClientServer/occlientbasicops %{ex_install_dir}
chrpath -d %{ex_install_dir}/occlientbasicops
cp out/linux/*/%{build_mode}/resource/csdk/stack/samples/linux/SimpleClientServer/occlientcoll %{ex_install_dir}
chrpath -d %{ex_install_dir}/occlientcoll
cp out/linux/*/%{build_mode}/resource/csdk/stack/samples/linux/SimpleClientServer/occlientslow %{ex_install_dir}
chrpath -d %{ex_install_dir}/occlientslow
cp out/linux/*/%{build_mode}/resource/csdk/stack/samples/linux/SimpleClientServer/ocserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/ocserver
cp out/linux/*/%{build_mode}/resource/csdk/stack/samples/linux/SimpleClientServer/ocserverbasicops %{ex_install_dir}
chrpath -d %{ex_install_dir}/ocserverbasicops
cp out/linux/*/%{build_mode}/resource/csdk/stack/samples/linux/SimpleClientServer/ocservercoll %{ex_install_dir}
chrpath -d %{ex_install_dir}/ocservercoll
cp out/linux/*/%{build_mode}/resource/csdk/stack/samples/linux/SimpleClientServer/ocserverslow %{ex_install_dir}
chrpath -d %{ex_install_dir}/ocserverslow
cp out/linux/*/%{build_mode}/service/resource-encapsulation/examples/linux/sampleResourceClient %{ex_install_dir}
chrpath -d %{ex_install_dir}/sampleResourceClient
cp out/linux/*/%{build_mode}/service/resource-encapsulation/examples/linux/sampleResourceServer %{ex_install_dir}
chrpath -d %{ex_install_dir}/sampleResourceServer
cp out/linux/*/%{build_mode}/service/resource-container/ContainerSample %{ex_install_dir}
chrpath -d %{ex_install_dir}/ContainerSample
cp out/linux/*/%{build_mode}/service/resource-hosting/SampleApp/linux/sampleprovider %{ex_install_dir}
chrpath -d %{ex_install_dir}/sampleprovider
cp out/linux/*/%{build_mode}/service/resource-hosting/SampleApp/linux/sampleconsumer %{ex_install_dir}
chrpath -d %{ex_install_dir}/sampleconsumer
cp out/linux/*/%{build_mode}/service/resource-hosting/SampleApp/linux/sampleresourcehosting %{ex_install_dir}
chrpath -d %{ex_install_dir}/sampleresourcehosting
cp out/linux/*/%{build_mode}/service/things-manager/sampleapp/linux/configuration/con-server %{ex_install_dir}
chrpath -d %{ex_install_dir}/con-server
cp out/linux/*/%{build_mode}/service/things-manager/sampleapp/linux/configuration/con-client %{ex_install_dir}
chrpath -d %{ex_install_dir}/con-client
cp out/linux/*/%{build_mode}/service/things-manager/sampleapp/linux/configuration/bootstrapserver %{ex_install_dir}
chrpath -d %{ex_install_dir}/bootstrapserver

cp out/linux/*/%{build_mode}/resource/examples/oic_svr_db_server.dat %{ex_install_dir}
cp out/linux/*/%{build_mode}/resource/examples/oic_svr_db_client.dat %{ex_install_dir}

%if 0%{?SECURED} == 1
mkdir -p %{ex_install_dir}/provisioning
mkdir -p %{ex_install_dir}/provision-sample

cp ./resource/csdk/security/include/pinoxmcommon.h %{buildroot}%{_includedir}
cp ./resource/csdk/security/provisioning/include/oxm/*.h %{buildroot}%{_includedir}
cp ./resource/csdk/security/provisioning/include/internal/*.h %{buildroot}%{_includedir}
cp ./resource/csdk/security/provisioning/include/*.h %{buildroot}%{_includedir}
cp ./resource/csdk/security/provisioning/sample/oic_svr_db_server_justworks.dat %{buildroot}%{_libdir}/oic_svr_db_server.dat
cp out/linux/*/%{build_mode}/service/resource-encapsulation/examples/linux/secureResourceExample/sampleAuthorizedClient %{ex_install_dir}
chrpath -d %{ex_install_dir}/sampleAuthorizedClient
cp out/linux/*/%{build_mode}/service/resource-encapsulation/examples/linux/secureResourceExample/sampleSecureServer %{ex_install_dir}
chrpath -d %{ex_install_dir}/sampleSecureServer
cp out/linux/*/%{build_mode}/service/resource-encapsulation/examples/linux/secureResourceExample/oic_svr_db_client.json %{ex_install_dir}
cp out/linux/*/%{build_mode}/service/resource-encapsulation/examples/linux/secureResourceExample/oic_svr_db_server.json %{ex_install_dir}
%endif

mkdir -p %{buildroot}%{_includedir}/service/resource-encapsulation
mkdir -p %{buildroot}%{_includedir}/service/resource-container
mkdir -p %{buildroot}%{_includedir}/service/resource-hosting
mkdir -p %{buildroot}%{_includedir}/service/resource-directory
mkdir -p %{buildroot}%{_includedir}/service/things-manager
mkdir -p %{buildroot}%{_includedir}/service/easy-setup
mkdir -p %{buildroot}%{_includedir}/resource

cp resource/csdk/stack/include/*.h %{buildroot}%{_includedir}/resource
cp resource/csdk/connectivity/inc/*.h %{buildroot}%{_includedir}/resource
cp resource/csdk/logger/include/*.h %{buildroot}%{_includedir}/resource
cp service/easy-setup/inc/*.h %{buildroot}%{_includedir}/service/easy-setup/
cp service/easy-setup/enrollee/inc/*.h %{buildroot}%{_includedir}/service/easy-setup/
cp service/resource-encapsulation/include/RCSRepresentation.h %{buildroot}%{_includedir}/service/resource-encapsulation/
cp service/resource-encapsulation/include/RCSSeparateResponse.h %{buildroot}%{_includedir}/service/resource-encapsulation/
cp service/resource-hosting/include/*.h %{buildroot}%{_includedir}/service/resource-hosting/
cp service/resource-directory/include/*.h %{buildroot}%{_includedir}/service/resource-directory/

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/liboc.so
%{_libdir}/liboc_logger.so
%{_libdir}/liboc_logger_core.so
%{_libdir}/liboctbstack.so
#%{_libdir}/libconnectivity_abstraction.so
%license LICENSE.APLv2

%files service
%defattr(-,root,root,-)
%{_libdir}/libBMISensorBundle.so
%{_libdir}/libDISensorBundle.so
#%{_libdir}/libresource_hosting.so
%{_libdir}/libTGMSDKLibrary.so
%{_libdir}/libHueBundle.so
%{_libdir}/librcs_client.so
%{_libdir}/librcs_common.so
%{_libdir}/librcs_container.so
%{_libdir}/librcs_server.so
%{_libdir}/libESEnrolleeSDK.so
%if 0%{?SECURED} == 1
%{_libdir}/libocpmapi.so
%{_libdir}/libocprovision.so
%{_libdir}/oic_svr_db_server.dat
%endif
%license LICENSE.APLv2

%files test
%defattr(-,root,root,-)
%{_bindir}/*
%license LICENSE.APLv2

%files devel
%defattr(-,root,root,-)
%license LICENSE.APLv2
%{_libdir}/lib*.a
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/*
