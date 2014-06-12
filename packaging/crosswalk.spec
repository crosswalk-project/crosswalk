%bcond_with x
%bcond_with wayland

%ifarch x86_64
# NaCl build on 64bit system require libc 32bit to build the 32 IRT.
# While Tizen 64bit image does not offer 32bit packages at all,
# check https://bugs.tizen.org/jira/browse/PTREL-803 for details.
# So disable nacl for 64bit now.
%define _disable_nacl 1
%else
%define _disable_nacl 0
%endif

Name:           crosswalk
Version:        7.36.153.0
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
Patch9:         Blink-Add-GCC-flag-Wno-narrowing-fix-64bits-build.patch
Patch10:        crosswalk-do-not-look-for-gtk-dependencies-on-x11.patch
Patch11:        crosswalk-tizen-ozonewl-xdgshell150.patch

BuildRequires:  binutils-gold
BuildRequires:  bison
BuildRequires:  bzip2-devel
BuildRequires:  elfutils
BuildRequires:  expat-devel
BuildRequires:  flex
BuildRequires:  gperf
BuildRequires:  libcap-devel
BuildRequires:  libelf-devel
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
BuildRequires:  pkgconfig(pkgmgr-installer)
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

%patch9

# The profiles using Wayland (and thus Ozone) do not need this patch.
%if !%{with wayland}
%patch10
%endif

%patch11

%build

# For ffmpeg on ia32. The original CFLAGS set by the gyp and config files in
# src/third_party/ffmpeg already pass -O2 -fomit-frame-pointer, but Tizen's
# CFLAGS end up appending -fno-omit-frame-pointer. See http://crbug.com/37246
export CFLAGS=`echo $CFLAGS | sed s,-fno-omit-frame-pointer,,g`

%if ! %{_disable_nacl}
# For nacl_bootstrap on ia32. The original CFLAGS set by the gyp
# native_client/src/trusted/service_runtime/linux/nacl_bootstrap.gyp already ignored 
# -fstack-protector and -funwind-tables, but Tizen's CFLAGS end up appending them, thus
# causing linking failures. Check XWALK-1689 for details.
export CFLAGS=`echo $CFLAGS | sed s,-fstack-protector,,g`
export CFLAGS=`echo $CFLAGS | sed s,-funwind-tables,,g`
%endif

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
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Duse_ozone=1 -Denable_ozone_wayland_vkb=1 -Denable_xdg_shell=1"
%endif

GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Ddisable_nacl=%{_disable_nacl}"

# Linking fails in Tizen when fatal ld warnings are enabled. XWALK-1379.
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Ddisable_fatal_linker_warnings=1"

# For building for arm in OBS, we need :
# -> to unset sysroot value.
# sysroot variable is automatically set for cross compilation to use arm-sysroot provided by Chromium project
# sysroot usage is not needed, we need to use arm libraries from the virtualized environment.
#
# Crosswalk build fails if the fpu selected in the gcc option is different from neon in case of arm7 compilation
# So force it.
%ifarch %{arm}
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Dsysroot="
export CFLAGS=`echo $CFLAGS | sed s,-mfpu=vfpv3,-mfpu=neon,g`
export CXXFLAGS=`echo $CXXFLAGS | sed s,-mfpu=vfpv3,-mfpu=neon,g`
export FFLAGS=`echo $FFLAGS | sed s,-mfpu=vfpv3,-mfpu=neon,g`
%endif

# --no-parallel is added because chroot does not mount a /dev/shm, this will
# cause python multiprocessing.SemLock error.
# Force gold binary from chroot ld.gold provided by binutils-gold
#  -Dlinux_use_bundled_binutils=0 -Dlinux_use_bundled_gold=0
export GYP_GENERATORS='ninja'
./src/xwalk/gyp_xwalk src/xwalk/xwalk.gyp \
--no-parallel \
${GYP_EXTRA_FLAGS} \
-Dchromeos=0 \
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
-Denable_hidpi=1 \
-Dlinux_use_bundled_binutils=0 \
-Dlinux_use_bundled_gold=0

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
install -p -D src/out/Release/libffmpegsumo.so %{buildroot}%{_libdir}/xwalk/libffmpegsumo.so
install -p -D src/out/Release/xwalk.pak %{buildroot}%{_libdir}/xwalk/xwalk.pak

# PNaCl
%if ! %{_disable_nacl}
install -p -D src/out/Release/libppGoogleNaClPluginChrome.so %{buildroot}%{_libdir}/xwalk/libppGoogleNaClPluginChrome.so
install -p -D src/out/Release/nacl_bootstrap_munge_phdr %{buildroot}%{_libdir}/xwalk/nacl_bootstrap_munge_phdr
install -p -D src/out/Release/nacl_bootstrap_raw %{buildroot}%{_libdir}/xwalk/nacl_bootstrap_raw
install -p -D src/out/Release/nacl_helper %{buildroot}%{_libdir}/xwalk/nacl_helper
install -p -D src/out/Release/nacl_helper_bootstrap %{buildroot}%{_libdir}/xwalk/nacl_helper_bootstrap
install -p -D src/out/Release/nacl_irt_*.nexe %{buildroot}%{_libdir}/xwalk
install -p -d %{buildroot}%{_libdir}/xwalk/pnacl
install -m 0664 -p -D src/out/Release/pnacl/* %{buildroot}%{_libdir}/xwalk/pnacl
%endif

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
%{_libdir}/xwalk/libffmpegsumo.so
%if ! %{_disable_nacl}
%{_libdir}/xwalk/libppGoogleNaClPluginChrome.so
%{_libdir}/xwalk/nacl_bootstrap_munge_phdr
%{_libdir}/xwalk/nacl_bootstrap_raw
%{_libdir}/xwalk/nacl_helper
%{_libdir}/xwalk/nacl_helper_bootstrap
%{_libdir}/xwalk/nacl_irt_*.nexe
%{_libdir}/xwalk/pnacl/*
%endif
%{_libdir}/xwalk/xwalk
%{_libdir}/xwalk/xwalk.pak
%{_manifestdir}/%{name}.xml
%{_desktop_icondir}/%{name}.png
%{_dbusservicedir}/org.crosswalkproject.Runtime1.service
%{_systemduserservicedir}/xwalk.service
