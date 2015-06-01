Name:           libtbm-vigs
Version:        0.1.0
Release:        2
License:        MIT
Summary:        Tizen Buffer Manager - emulator backend
Group:          System/Libraries
Source0:        %{name}-%{version}.tar.gz
Source1001:     libtbm-vigs.manifest

BuildRequires:  pkgconfig(pthread-stubs)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libtbm)

Requires:   libtbm
Requires:   libdrm

%description
description: ${summary}

%prep
%setup -q
cp %{SOURCE1001} .

%build
autoreconf -vfi
./configure --prefix=%{_prefix} --libdir=%{_libdir}/bufmgr \
            CFLAGS="${CFLAGS} -Wall -Werror" LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--as-needed"

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp -af COPYING %{buildroot}/usr/share/license/%{name}

%make_install

%post
if [ -f %{_libdir}/bufmgr/libtbm_default.so ]; then
    rm -rf %{_libdir}/bufmgr/libtbm_default.so
fi
ln -s libtbm_emulator.so %{_libdir}/bufmgr/libtbm_default.so

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
/usr/share/license/%{name}
%{_libdir}/bufmgr/libtbm_*.so*
