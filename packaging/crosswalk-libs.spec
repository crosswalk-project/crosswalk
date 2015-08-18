%bcond_with x
%bcond_with wayland

%ifarch x86_64 %{arm}
### x86_64
# NaCl build on 64bit system require libc 32bit to build the 32 IRT.
# While Tizen 64bit image does not offer 32bit packages at all,
# check https://bugs.tizen.org/jira/browse/PTREL-803 for details.
# So disable nacl for 64bit now.
### ARM
# Due to OBS build for ARM some files needed by NaCl to be build
# are not present.
%define _disable_nacl 1
%else
# Since M39, Google has stopped shipping a 32-bit PNaCl toolchain, so we cannot
# build NaCl on a fully 32-bit host anymore. See XWALK-2679.
%define _disable_nacl 1
%endif

# adjust compression algorithm to speed up RPMS creation
# source RPM and debug RPMS are big and take too much time
# when using standard (lzma) compression
%define _source_payload w3.gzdio
%define _binary_payload w3.gzdio

Name:           crosswalk-libs
Version:        16.44.390.0
Release:        0
Summary:        Chromium-based app runtime, private libraries
License:        (BSD-3-Clause and LGPL-2.1+)
Group:          Web Framework/Web Run Time
Url:            https://github.com/otcshare/crosswalk
Source:         crosswalk.tar
Source1001:     crosswalk-libs.manifest
Source1002:     print-chromium-deps.py
Patch10:        crosswalk-do-not-look-for-gtk-dependencies-on-x11.patch

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
BuildRequires:  yasm
BuildRequires:  pkgconfig(alsa)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libpci)
BuildRequires:  pkgconfig(libpulse)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(libxslt)
BuildRequires:  pkgconfig(pango)
BuildRequires:  pkgconfig(protobuf)
BuildRequires:  pkgconfig(nss)

%if %{with wayland}
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(wayland-cursor)
BuildRequires:  pkgconfig(wayland-egl)
BuildRequires:  pkgconfig(xkbcommon)
%endif

%if %{with x}
BuildRequires:  pkgconfig(scim)
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

%if "%{profile}" == "ivi"
BuildRequires:  pkgconfig(murphy-common)
BuildRequires:  pkgconfig(murphy-resource)
%endif

%description
Crosswalk is an app runtime based on Chromium. It is an open source project started by the Intel Open Source Technology Center (http://www.01.org). This package contains private shared libraries required for running Crosswalk.

%prep
%setup -q -n crosswalk

cp %{SOURCE1001} .

cp -a src/AUTHORS AUTHORS.chromium
cp -a src/LICENSE LICENSE.chromium
cp -a src/xwalk/LICENSE LICENSE.xwalk

# The profiles using Wayland (and thus Ozone) do not need this patch.
%if !%{with wayland}
%patch10
%endif

%build

# Stop unconditionally passing -Wall to the compiler. Chromium has its own
# mechanisms for deciding which parts of the code need -Wall and which need it
# to be left out (since several pieces are built with -Werror). At least in
# M39, this is preventing the "rtc_base" target from being built because it
# does not expect -Wall to be passed to the compiler (see webrtc issue 3307).
export CXXFLAGS=`echo $CXXFLAGS | sed s,-Wall,,g`

# Do not use -finline-functions: it breaks the build because it causes -Wall to
# warn about some conditions that cannot really be reached (ie. variables that
# may be used uninitialized while in fact thay cannot be uninitialized). See
# TC-2299.
export CXXFLAGS=`echo $CXXFLAGS | sed s,-finline-functions,,g`

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
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Duse_ozone=1 -Duse_xkbcommon=1"
%endif

GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Ddisable_nacl=%{_disable_nacl}"

# Linking fails when fatal ld warnings are enabled. See XWALK-1379.
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Ddisable_fatal_linker_warnings=1"

# For building for arm in OBS, we need :
# -> to unset sysroot value.
# sysroot variable is automatically set for cross compilation to use arm-sysroot provided by Chromium project
# sysroot usage is not needed, we need to use arm libraries from the virtualized environment.
#
# Crosswalk build fails if the fpu selected in the gcc option is different from neon in case of arm7 compilation
# So force it.
%ifarch %{arm}
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Dsysroot= "
export CFLAGS=`echo $CFLAGS | sed s,-mfpu=vfpv3,-mfpu=neon,g`
export CXXFLAGS=`echo $CXXFLAGS | sed s,-mfpu=vfpv3,-mfpu=neon,g`
export FFLAGS=`echo $FFLAGS | sed s,-mfpu=vfpv3,-mfpu=neon,g`
%endif

%if "%{profile}" == "ivi"
GYP_EXTRA_FLAGS="${GYP_EXTRA_FLAGS} -Denable_murphy=1"
%endif

export GYP_GENERATORS="dump_dependency_json,ninja"

# --no-parallel is added because chroot does not mount a /dev/shm, this will
# cause python multiprocessing.SemLock error.
./src/xwalk/gyp_xwalk src/xwalk/xwalk.gyp \
--no-parallel \
${GYP_EXTRA_FLAGS} \
-Dbuilding_crosswalk_bin=0 \
-Dchromeos=0 \
-Dclang=0 \
-Dcomponent=shared_library \
-Dlinux_use_bundled_binutils=0 \
-Dlinux_use_bundled_gold=0 \
-Dlinux_use_gold_flags=1 \
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
-Duse_system_protobuf=1 \
-Duse_system_yasm=1 \
-Denable_hidpi=1

# We are not interested in building ${NINJA_TARGETS} themselves, since that is
# what crosswalk-bin.spec does. We want the Chromium dependencies that those
# targets need to build all the shared libraries we have to package.
NINJA_TARGETS="xwalk xwalk_application_tools"
DEPENDENCIES=`python %{SOURCE1002} dump.json ${NINJA_TARGETS}`

ninja %{?_smp_mflags} -C src/out/Release ${DEPENDENCIES}

# In '%files' section below there are added all built libraries, which satisfy
# pattern: 'xwalk/lib/lib*.so'. During build crosswalk-bin.spec generates new
# library - libxwalk_backend_lib.so, which should not be packaged into
# crosswalk-libs, but then after incremental build it will also satisfy
# pattern, which was mentioned above.
rm -f src/out/Release/lib/libxwalk_backend_lib.so

%install
# Supporting libraries and resources.
install -d %{buildroot}%{_libdir}/xwalk/lib
install -m 0644 -p -D src/out/Release/lib/*.so %{buildroot}%{_libdir}/xwalk/lib/
install -m 0644 -p -D src/out/Release/icudtl.dat %{buildroot}%{_libdir}/xwalk/icudtl.dat
install -m 0644 -p -D src/out/Release/natives_blob.bin %{buildroot}%{_libdir}/xwalk/natives_blob.bin
install -m 0644 -p -D src/out/Release/snapshot_blob.bin %{buildroot}%{_libdir}/xwalk/snapshot_blob.bin

%files
%manifest crosswalk-libs.manifest
%{_libdir}/xwalk/icudtl.dat
%{_libdir}/xwalk/lib/lib*.so
%if ! %{_disable_nacl}
%{_libdir}/xwalk/nacl_bootstrap_raw
%{_libdir}/xwalk/nacl_helper
%{_libdir}/xwalk/nacl_helper_bootstrap
%{_libdir}/xwalk/nacl_irt_*.nexe
%{_libdir}/xwalk/pnacl/*
%endif
%{_libdir}/xwalk/natives_blob.bin
%{_libdir}/xwalk/snapshot_blob.bin
