%bcond_with x
%bcond_with wayland

Name:           crosswalk
Version:        6.35.120.0
Release:        0
Summary:        Crosswalk is an app runtime based on Chromium
License:        (BSD-3-Clause and LGPL-2.1+)
Group:          Web Framework/Web Run Time
Url:            https://github.com/otcshare/crosswalk
Source:         %{name}.tar
Source1:        xwalk.in
Source2:        org.crosswalkproject.Runtime1.service
Source3:        xwalk.service.in
Source1001:     crosswalk.manifest
Source1002:     %{name}.xml.in
Source1003:     %{name}.png
Patch1:         %{name}-do-not-look-for-gtk2-when-using-aura.patch
Patch2:         %{name}-no-fatal-ld-warnings.patch
Patch9:         Blink-Add-GCC-flag-Wno-narrowing-fix-64bits-build.patch

BuildRequires:  bison
BuildRequires:  bzip2-devel
BuildRequires:  expat-devel
BuildRequires:  flex
BuildRequires:  gperf
BuildRequires:  libcap-devel
BuildRequires:  ninja
BuildRequires:  python
BuildRequires:  python-xml
BuildRequires:  perl
BuildRequires:  which
BuildRequires:  pkgconfig(alsa)
BuildRequires:  pkgconfig(appcore-common)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(capi-location-manager)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(haptic)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libpci)
BuildRequires:  pkgconfig(libpulse)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(libxslt)
BuildRequires:  pkgconfig(pango)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(pkgmgr-parser)
BuildRequires:  pkgconfig(nspr)
BuildRequires:  pkgconfig(nss)
BuildRequires:  pkgconfig(sensor)
BuildRequires:  pkgconfig(vconf)
%if %{with x}
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
%endif

%if %{with wayland}
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(wayland-cursor)
BuildRequires:  pkgconfig(wayland-egl)
BuildRequires:  pkgconfig(xkbcommon)
%else
BuildRequires:  pkgconfig(scim)
%endif

%description
Crosswalk is an app runtime based on Chromium. It is an open source project started by the Intel Open Source Technology Center (http://www.01.org).

%define _manifestdir /usr/share/packages
%define _manifestdir_ro /opt/share/packages
%define _desktop_icondir /usr/share/icons/default/small
%define _desktop_icondir_ro /opt/share/icons/default/small
%define _dbusservicedir /usr/share/dbus-1/services
%define _systemduserservicedir /usr/lib/systemd/user

%prep
%setup -q -n crosswalk

cp %{SOURCE1} .
cp %{SOURCE3} .
cp %{SOURCE1001} .
cp %{SOURCE1002} .
cp %{SOURCE1003} .
sed "s/@VERSION@/%{version}/g" %{name}.xml.in > %{name}.xml
sed "s|@LIB_INSTALL_DIR@|%{_libdir}|g" xwalk.in > xwalk
sed "s|@LIB_INSTALL_DIR@|%{_libdir}|g" xwalk.service.in > xwalk.service

cp -a src/AUTHORS AUTHORS.chromium
cp -a src/LICENSE LICENSE.chromium
cp -a src/xwalk/LICENSE LICENSE.xwalk

%patch1

# Linking fails in Tizen Common when fatal ld warnings are enabled. XWALK-1379.
%if "%{profile}" == "common" || "%{profile}" == "generic"
%patch2
%endif

%patch9

%build

# For ffmpeg on ia32. The original CFLAGS set by the gyp and config files in
# src/third_party/ffmpeg already pass -O2 -fomit-frame-pointer, but Tizen's
# CFLAGS end up appending -fno-omit-frame-pointer. See http://crbug.com/37246
export CFLAGS=`echo $CFLAGS | sed s,-fno-omit-frame-pointer,,g`

# Building the RPM in the GBS chroot fails with errors such as
#   /usr/lib/gcc/i586-tizen-linux/4.7/../../../../i586-tizen-linux/bin/ld:
#       failed to set dynamic section sizes: Memory exhausted
# For now, work around it by passing a GNU ld-specific flag that optimizes the
# linker for memory usage.
export LDFLAGS="${LDFLAGS} -Wl,--no-keep-memory"

# Support building in a non-standard directory, possibly outside %{_builddir}.
# Since the build root is erased every time a new build is performed, one way
# to avoid losing the build directory is to specify a location outside the
# build root to the BUILDDIR_NAME definition, such as "/var/tmp/xwalk-build"
# (remember all paths are still inside the chroot):
#    gbs build --define 'BUILDDIR_NAME /some/path'
BUILDDIR_NAME="%{?BUILDDIR_NAME}"
if [ -n "${BUILDDIR_NAME}" ]; then
   mkdir -p "${BUILDDIR_NAME}"
   ln -s "${BUILDDIR_NAME}" src/out
fi

%if %{with wayland}
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Duse_ozone=1 -Denable_ozone_wayland_vkb=1"
%endif

# --no-parallel is added because chroot does not mount a /dev/shm, this will
# cause python multiprocessing.SemLock error.
export GYP_GENERATORS='ninja'
./src/xwalk/gyp_xwalk src/xwalk/xwalk.gyp \
--no-parallel \
${GYP_EXTRA_FLAGS} \
-Dchromeos=0 \
-Ddisable_nacl=1 \
-Dtizen=1 \
-Dpython_ver=2.7 \
-Duse_aura=1 \
-Duse_cups=0 \
-Duse_gconf=0 \
-Duse_gnome_keyring=0 \
-Duse_kerberos=0 \
-Duse_system_bzip2=1 \
-Duse_system_libexif=1 \
-Duse_system_libxml=1 \
-Duse_system_nspr=1 \
-Denable_hidpi=1

ninja %{?_smp_mflags} -C src/out/Release xwalk xwalkctl xwalk_launcher xwalk-pkg-helper

%install
# Binaries.
install -p -D xwalk %{buildroot}%{_bindir}/xwalk
install -p -D %{SOURCE2} %{buildroot}%{_dbusservicedir}/org.crosswalkproject.Runtime1.service
install -p -D xwalk.service %{buildroot}%{_systemduserservicedir}/xwalk.service
install -p -D src/out/Release/xwalk %{buildroot}%{_libdir}/xwalk/xwalk
install -p -D src/out/Release/xwalkctl %{buildroot}%{_bindir}/xwalkctl
install -p -D src/out/Release/xwalk-launcher %{buildroot}%{_bindir}/xwalk-launcher
# xwalk-pkg-helper needs to be set-user-ID-root so it can finish the installation process.
install -m 06755 -p -D src/out/Release/xwalk-pkg-helper %{buildroot}%{_bindir}/xwalk-pkg-helper

# Supporting libraries and resources.
install -p -D src/out/Release/icudtl.dat %{buildroot}%{_libdir}/xwalk/icudtl.dat
install -p -D src/out/Release/lib/libmojo_system.so %{buildroot}%{_libdir}/xwalk/lib/libmojo_system.so
install -p -D src/out/Release/libffmpegsumo.so %{buildroot}%{_libdir}/xwalk/libffmpegsumo.so
install -p -D src/out/Release/xwalk.pak %{buildroot}%{_libdir}/xwalk/xwalk.pak

# Register xwalk to the package manager.
install -p -D %{name}.xml %{buildroot}%{_manifestdir}/%{name}.xml
install -p -D %{name}.png %{buildroot}%{_desktop_icondir}/%{name}.png

%post
mkdir -p %{_desktop_icondir_ro}
mkdir -p %{_manifestdir_ro}

%files
%manifest %{name}.manifest
%license AUTHORS.chromium LICENSE.chromium LICENSE.xwalk
%{_bindir}/xwalk
%{_bindir}/xwalkctl
%{_bindir}/xwalk-launcher
%{_bindir}/xwalk-pkg-helper
%{_libdir}/xwalk/icudtl.dat
%{_libdir}/xwalk/lib/libmojo_system.so
%{_libdir}/xwalk/libffmpegsumo.so
%{_libdir}/xwalk/xwalk
%{_libdir}/xwalk/xwalk.pak
%{_manifestdir}/%{name}.xml
%{_desktop_icondir}/%{name}.png
%{_dbusservicedir}/org.crosswalkproject.Runtime1.service
%{_systemduserservicedir}/xwalk.service
