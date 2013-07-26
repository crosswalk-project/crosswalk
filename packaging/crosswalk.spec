Name:           crosswalk
Version:        0.28.0.1
Release:        0
Summary:        Crosswalk is an app runtime based on Chromium
# License:        (BSD-3-Clause and LGPL-2.1+)
License:        BSD-3-Clause
Group:          Web Framework/Web Run Time
Url:            https://github.com/otcshare/crosswalk
Source:         %{name}-%{version}.tar
Source1:        xwalk
Source1001:     crosswalk.manifest
Patch1:         crosswalk-0.28.0.1-do-not-look-for-gtk2-when-using-aura.patch

BuildRequires:  bison
BuildRequires:  bzip2-devel
BuildRequires:  expat-devel
BuildRequires:  flex
BuildRequires:  gperf
BuildRequires:  libasound-devel
BuildRequires:  python
BuildRequires:  python-xml
BuildRequires:  perl
BuildRequires:  which
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libpci)
BuildRequires:  pkgconfig(libpulse)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(libxslt)
BuildRequires:  pkgconfig(pango)
BuildRequires:  pkgconfig(nspr)
BuildRequires:  pkgconfig(nss)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xcomposite)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xrandr)
BuildRequires:  pkgconfig(xrender)
BuildRequires:  pkgconfig(xscrnsaver)
BuildRequires:  pkgconfig(xt)
BuildRequires:  pkgconfig(xtst)

%description
Crosswalk is an app runtime based on Chromium. It is an open source project started by the Intel Open Source Technology Center (http://www.01.org).

%package internal-devel
Summary:        Tools and development headers used by extension writers
License:        BSD-3-Clause
Group:          Development/Libraries
Requires:       %{name} = %{version}

%description internal-devel
The crosswalk-internal-devel package contains code generation tools and development headers needed by third parties who wish to write their own Crosswalk extensions.

%prep
%setup -q

cp %{SOURCE1001} .

cp -a src/AUTHORS AUTHORS.chromium
cp -a src/LICENSE LICENSE.chromium
cp -a src/xwalk/AUTHORS AUTHORS.xwalk
cp -a src/xwalk/LICENSE LICENSE.xwalk

%patch1

%build

# For ffmpeg on ia32. The original CFLAGS set by the gyp and config files in
# src/third_party/ffmpeg already pass -O2 -fomit-frame-pointer, but Tizen's
# CFLAGS end up appending -fno-omit-frame-pointer. See http://crbug.com/37246
export CFLAGS=`echo $CFLAGS | sed s,-fno-omit-frame-pointer,,g`

export GYP_GENERATORS='make'
./src/xwalk/gyp_xwalk src/xwalk/xwalk.gyp \
-Ddisable_nacl=1 \
-Duse_aura=1 \
-Duse_cups=0 \
-Duse_gconf=0 \
-Duse_kerberos=0 \
-Duse_system_bzip2=1 \
-Duse_system_icu=1 \
-Duse_system_libexif=1 \
-Duse_system_libxml=1 \
-Duse_system_nspr=1 \
-Duse_xi2_mt=2 \

make %{?_smp_mflags} -C src BUILDTYPE=Release xwalk

%install
# Binaries.
install -m 755 -D %{SOURCE1} %{buildroot}%{_bindir}/xwalk
install -m 755 -D src/out/Release/xwalk %{buildroot}%{_libdir}/xwalk/xwalk

# Supporting libraries and resources.
install -m 644 -D src/out/Release/libffmpegsumo.so %{buildroot}%{_libdir}/xwalk/libffmpegsumo.so
install -m 644 -D src/out/Release/xwalk.pak %{buildroot}%{_libdir}/xwalk/xwalk.pak

# Development files and tools.
install -m 644 -D src/xwalk/extensions/public/xwalk_extension_public.h %{buildroot}%{_includedir}/xwalk/extensions/public/xwalk_extension_public.h
# TODO(rakuco): This should not be in %{_includedir}, but rather in %{_libexecdir}.
install -m 755 -D src/xwalk/extensions/tools/generate_api.py %{buildroot}%{_includedir}/xwalk/extensions/tools/generate_api.py

%files
%manifest %{name}.manifest
# %license AUTHORS.chromium AUTHORS.xwalk LICENSE.chromium LICENSE.xwalk
%{_bindir}/xwalk
%{_libdir}/xwalk/libffmpegsumo.so
%{_libdir}/xwalk/xwalk
%{_libdir}/xwalk/xwalk.pak

%files internal-devel
%manifest %{name}.manifest
%{_includedir}/xwalk/*
