Name:           crosswalk
Version:        1.28.2.0
Release:        0
Summary:        Crosswalk is an app runtime based on Chromium
# License:        (BSD-3-Clause and LGPL-2.1+)
License:        BSD-3-Clause
Group:          Web Framework/Web Run Time
Url:            https://github.com/otcshare/crosswalk
Source:         %{name}-%{version}.tar
Source1:        xwalk
Source1001:     crosswalk.manifest
Source1002:     %{name}.xml.in
Source1003:     %{name}.png
Patch1:         %{name}-1.28-do-not-look-for-gtk2-when-using-aura.patch
Patch2:         %{name}-1.28-look-for-pvr-libGLESv2.so.patch
Patch3:         %{name}-1.28-revert-chromium-r209481.patch

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
BuildRequires:  pkgconfig(gles20)
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

%package emulator-support
Summary:        Support files necessary for running Crosswalk on the Tizen emulator
# License:        (BSD-3-Clause and LGPL-2.1+)
License:        BSD-3-Clause
Group:          Web Framework/Web Run Time
Url:            https://github.com/otcshare/crosswalk

%description emulator-support
This package contains additional support files that are needed for running Crosswalk on the Tizen emulator.

%define _manifestdir /usr/share/packages
%define _desktop_icondir /usr/share/icons/default/small

%prep
%setup -q

cp %{SOURCE1001} .
cp %{SOURCE1002} .
cp %{SOURCE1003} .
sed "s/@VERSION@/%{version}/g" %{name}.xml.in > %{name}.xml

cp -a src/AUTHORS AUTHORS.chromium
cp -a src/LICENSE LICENSE.chromium
cp -a src/xwalk/AUTHORS AUTHORS.xwalk
cp -a src/xwalk/LICENSE LICENSE.xwalk

%patch1
%patch2
%patch3

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
-Denable_xi21_mt=1 \
-Dtizen_mobile=1

make %{?_smp_mflags} -C src BUILDTYPE=Release xwalk

%install
# Binaries.
install -p -D %{SOURCE1} %{buildroot}%{_bindir}/xwalk
install -p -D src/out/Release/xwalk %{buildroot}%{_libdir}/xwalk/xwalk

# Supporting libraries and resources.
install -p -D src/out/Release/libffmpegsumo.so %{buildroot}%{_libdir}/xwalk/libffmpegsumo.so
install -p -D src/out/Release/libosmesa.so %{buildroot}%{_libdir}/xwalk/libosmesa.so
install -p -D src/out/Release/xwalk.pak %{buildroot}%{_libdir}/xwalk/xwalk.pak

# Register xwalk to the package manager.
install -p -D %{name}.xml %{buildroot}%{_manifestdir}/%{name}.xml
install -p -D %{name}.png %{buildroot}%{_desktop_icondir}/%{name}.png

%files
%manifest %{name}.manifest
# %license AUTHORS.chromium AUTHORS.xwalk LICENSE.chromium LICENSE.xwalk
%{_bindir}/xwalk
%{_libdir}/xwalk/libffmpegsumo.so
%{_libdir}/xwalk/xwalk
%{_libdir}/xwalk/xwalk.pak
%{_manifestdir}/%{name}.xml
%{_desktop_icondir}/%{name}.png

%files emulator-support
%{_libdir}/xwalk/libosmesa.so
