Name: rdma-plumbing
Version: 1
Release: 1%{?dist}
Summary: Userspace components for the Linux Kernel\'s drivers/infiniband stack

License: GPLv2 or BSD
Url: http://openfabrics.org/
Source: rdma-plumbing-%{version}.tgz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: binutils
BuildRequires: cmake >= 2.8.11
BuildRequires: gcc
BuildRequires: pkgconfig
BuildRequires: pkgconfig(libnl-3.0)
BuildRequires: pkgconfig(libnl-route-3.0)

# Since we recommend developers use Ninja, so should packagers, for consistency.
%define CMAKE_FLAGS %{nil}
%if 0%{?suse_version}
# SuSE releases have it, and sometime around cmake 3.3.2-1.2 the macros learned to use it.
BuildRequires: ninja,make
%define __builder ninja
# cmake_install,make_jobs is specified by opensuse
%else
%if 0%{?fedora} >= 23
# Ninja was introduced in FC23
BuildRequires: ninja-build
%define CMAKE_FLAGS -GNinja
%define make_jobs ninja -v %{?_smp_mflags}
%define cmake_install DESTDIR=%{buildroot} ninja-build install
%else
# Fallback to make otherwise
BuildRequires: make
%define make_jobs make -v %{?_smp_mflags}
%define cmake_install DESTDIR=%{buildroot} make install
%endif
%endif

%description
Temporary packaging

This is a simple example without the split sub packages to get things started.

%prep
%setup

%build

# Detect if systemd is supported on this system
%if 0%{?_unitdir:1}
%define my_unitdir %{_unitdir}
%else
%define my_unitdir /tmp/
%endif

# Pass all of the rpm paths directly to GNUInstallDirs and our other defines.
%cmake %{CMAKE_FLAGS} \
         -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_BINDIR:PATH=%{_bindir} \
         -DCMAKE_INSTALL_SBINDIR:PATH=%{_sbindir} \
         -DCMAKE_INSTALL_LIBDIR:PATH=%{_libdir} \
         -DCMAKE_INSTALL_LIBEXECDIR:PATH=%{_libexecdir} \
         -DCMAKE_INSTALL_LOCALSTATEDIR:PATH=%{_localstatedir} \
         -DCMAKE_INSTALL_SHAREDSTATEDIR:PATH=%{_sharedstatedir} \
         -DCMAKE_INSTALL_INCLUDEDIR:PATH=%{_includedir} \
         -DCMAKE_INSTALL_INFODIR:PATH=%{_infodir} \
         -DCMAKE_INSTALL_MANDIR:PATH=%{_mandir} \
         -DCMAKE_INSTALL_SYSCONFDIR:PATH=%{_sysconfdir} \
	 -DCMAKE_INSTALL_SYSTEMD_SERVICEDIR:PATH=%{my_unitdir} \
	 -DCMAKE_INSTALL_INITDDIR:PATH=%{_initrddir}
%make_jobs

%install
%cmake_install

%if 0%{?_unitdir:1}
rm -rf %{buildroot}/%{_initrddir}/
%else
rm -rf %{buildroot}/%{my_unitdir}/
%endif

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%doc %{_mandir}/man*/*
%{_bindir}/*
%{_includedir}/*
%{_libdir}/lib*
%{_libdir}/rsocket/*
%{_sbindir}/*
%if 0%{?_unitdir:1}
%{_unitdir}/*
%else
%config %{_initrddir}/*
%endif
%config %{_sysconfdir}/iwpmd.conf
%config %{_sysconfdir}/srp_daemon.conf
%config %{_sysconfdir}/libibverbs.d/*
%config %{_sysconfdir}/logrotate.d/srp_daemon
%{_sysconfdir}/modprobe.d/*
%config %{_sysconfdir}/rsyslog.d/srp_daemon.conf
