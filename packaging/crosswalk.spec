Name:           crosswalk
Version:        4.32.76.4
Release:        0
Summary:        Crosswalk is an app runtime based on Chromium
# License:        (BSD-3-Clause and LGPL-2.1+)
License:        BSD-3-Clause
Group:          Web Framework/Web Run Time
Url:            https://github.com/otcshare/crosswalk
Source:         %{name}.tar
Source1:        xwalk
Source2:        org.crosswalkproject.Runtime1.service
Source3:        xwalk.service
Source1001:     crosswalk.manifest
Source1002:     %{name}.xml.in
Source1003:     %{name}.png
Patch1:         %{name}-do-not-look-for-gtk2-when-using-aura.patch
Patch2:         %{name}-look-for-pvr-libGLESv2.so.patch
Patch3:         %{name}-include-tizen-ime-files.patch
Patch4:         %{name}-disable-ffmpeg-pragmas.patch
Patch5:         Chromium-Fix-gcc-4.5.3-uninitialized-warnings.patch
Patch6:         Blink-Fix-gcc-4.5.3-uninitialized-warnings.patch
Patch7:         %{name}-tizen-audio-session-manager.patch
Patch8:         %{name}-mesa-ozone-typedefs.patch

BuildRequires:  bison
BuildRequires:  bzip2-devel
BuildRequires:  expat-devel
BuildRequires:  flex
BuildRequires:  gperf
BuildRequires:  libcap-devel
BuildRequires:  python
BuildRequires:  python-xml
BuildRequires:  perl
BuildRequires:  which
BuildRequires:  pkgconfig(alsa)
BuildRequires:  pkgconfig(appcore-common)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(audio-session-mgr)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(capi-appfw-application)
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
BuildRequires:  pkgconfig(sensor)
BuildRequires:  pkgconfig(vconf)
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

# Depending on the Tizen version and profile we are building for, we have
# different dependencies, patches and gyp options to pass. Checking for
# specific profiles is not very future-proof. We therefore try to check for
# either specific features that may be enabled in the current profile (such as
# Wayland support) or for a certain Tizen major version (the differences betwen
# Tizen 2 and Tizen 3 are big enough that we need completely different patches
# and build dependencies, for example).
%bcond_with wayland

%if "%{tizen}" < "3.0"
BuildRequires:  gst-plugins-atomisp-devel
BuildRequires:  pkgconfig(openssl)
%else
BuildRequires:  pkgconfig(nss)
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
%define _dbusservicedir /usr/share/dbus-1/services
%define _systemduserservicedir /usr/lib/systemd/user

%prep
%setup -q -n crosswalk

cp %{SOURCE1001} .
cp %{SOURCE1002} .
cp %{SOURCE1003} .
sed "s/@VERSION@/%{version}/g" %{name}.xml.in > %{name}.xml

cp -a src/AUTHORS AUTHORS.chromium
cp -a src/LICENSE LICENSE.chromium
cp -a src/xwalk/AUTHORS AUTHORS.xwalk
cp -a src/xwalk/LICENSE LICENSE.xwalk

%patch1
%patch7

%if "%{tizen}" < "3.0"
%patch2
%patch3
%patch4
%patch5 -p1
%patch6 -p1
%endif

%if %{with wayland}
%patch8
%endif

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
#
# The --depth and --generator-output combo is used to put all the Makefiles
# inside the build directory, and (this is the important part) keep file lists
# (generatedwith <|() in gyp) in the build directory as well, otherwise they
# will be in the source directory, erased every time and trigger an almost full
# Blink rebuild (among other smaller targets).
# We cannot always pass those flags, though, because gyp's make generator does
# not work if the --generator-output is the top-level source directory.
BUILDDIR_NAME="%{?BUILDDIR_NAME}"
if [ -z "${BUILDDIR_NAME}" ]; then
   BUILDDIR_NAME="."
else
   GYP_EXTRA_FLAGS="--depth=. --generator-output=${BUILDDIR_NAME}"
fi

# Tizen 2's NSS is too old for Chromium, so we have to use the OpenSSL backend.
%if "%{tizen}" < "3.0"
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Dtizen_mobile=1 -Duse_openssl=1"
%endif

%if %{with wayland}
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Duse_ash=1 -Duse_ozone=1"
%endif

# Change src/ so that we can pass "." to --depth below, otherwise we would need
# to pass "src" to it, but this confuses the gyp make generator, that expects
# to be called from the root source directory.
cd src

# --no-parallel is added because chroot does not mount a /dev/shm, this will
# cause python multiprocessing.SemLock error.
export GYP_GENERATORS='make'
./xwalk/gyp_xwalk xwalk/xwalk.gyp \
--no-parallel \
${GYP_EXTRA_FLAGS} \
-Dchromeos=0 \
-Ddisable_nacl=1 \
-Dpython_ver=2.7 \
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
-Duse_xi2_mt=0

make %{?_smp_mflags} -C "${BUILDDIR_NAME}" BUILDTYPE=Release xwalk xwalkctl xwalk_launcher xwalk-pkg-helper

%install
# Support building in a non-standard directory, possibly outside %{_builddir}.
# Since the build root is erased every time a new build is performed, one way
# to avoid losing the build directory is to specify a location outside the
# build root to the BUILDDIR_NAME definition, such as "/var/tmp/xwalk-build"
# (remember all paths are still inside the chroot):
#    gbs build --define 'BUILDDIR_NAME /some/path'
BUILDDIR_NAME="%{?BUILDDIR_NAME}"
if [ -z "${BUILDDIR_NAME}" ]; then
   BUILDDIR_NAME="."
fi

# Since BUILDDIR_NAME can be either a relative path or an absolute one, we need
# to cd into src/ so that it means the same thing in the build and install
# stages: during the former, a relative location refers to a place inside src/,
# whereas during the latter a relative location by default would refer to a
# place one directory above src/. If BUILDDIR_NAME is an absolute path, this is
# irrelevant anyway.
cd src

# Binaries.
install -p -D %{SOURCE1} %{buildroot}%{_bindir}/xwalk
install -p -D %{SOURCE2} %{buildroot}%{_dbusservicedir}/org.crosswalkproject.Runtime1.service
install -p -D %{SOURCE3} %{buildroot}%{_systemduserservicedir}/xwalk.service
install -p -D ${BUILDDIR_NAME}/out/Release/xwalk %{buildroot}%{_libdir}/xwalk/xwalk
install -p -D ${BUILDDIR_NAME}/out/Release/xwalkctl %{buildroot}%{_bindir}/xwalkctl
install -p -D ${BUILDDIR_NAME}/out/Release/xwalk-launcher %{buildroot}%{_bindir}/xwalk-launcher
# xwalk-pkg-helper needs to be set-user-ID-root so it can finish the installation process.
install -m 06755 -p -D ${BUILDDIR_NAME}/out/Release/xwalk-pkg-helper %{buildroot}%{_bindir}/xwalk-pkg-helper

# Supporting libraries and resources.
install -p -D ${BUILDDIR_NAME}/out/Release/libffmpegsumo.so %{buildroot}%{_libdir}/xwalk/libffmpegsumo.so
install -p -D ${BUILDDIR_NAME}/out/Release/libosmesa.so %{buildroot}%{_libdir}/xwalk/libosmesa.so
install -p -D ${BUILDDIR_NAME}/out/Release/xwalk.pak %{buildroot}%{_libdir}/xwalk/xwalk.pak

# Register xwalk to the package manager.
install -p -D ../%{name}.xml %{buildroot}%{_manifestdir}/%{name}.xml
install -p -D ../%{name}.png %{buildroot}%{_desktop_icondir}/%{name}.png

%files
%manifest %{name}.manifest
# %license AUTHORS.chromium AUTHORS.xwalk LICENSE.chromium LICENSE.xwalk
%{_bindir}/xwalk
%{_bindir}/xwalkctl
%{_bindir}/xwalk-launcher
%{_bindir}/xwalk-pkg-helper
%{_libdir}/xwalk/libffmpegsumo.so
%{_libdir}/xwalk/xwalk
%{_libdir}/xwalk/xwalk.pak
%{_manifestdir}/%{name}.xml
%{_desktop_icondir}/%{name}.png
%{_dbusservicedir}/org.crosswalkproject.Runtime1.service
%{_systemduserservicedir}/xwalk.service

%files emulator-support
%{_libdir}/xwalk/libosmesa.so
