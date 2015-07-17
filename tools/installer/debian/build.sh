#!/bin/bash
#
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is a tuned-down version of the scripts used to build Chrome Debian
# packages. See src/chrome/installer/linux/debian for the original.

set -e
set -o pipefail
if [ "$VERBOSE" ]; then
  set -x
fi
set -u

# Create the Debian changelog file needed by dpkg-gencontrol.
# There is some sed/awk trickery involved to get the git commit log formatted
# and appended to the regular list of changes.
gen_changelog() {
  rm -f "${DEB_CHANGELOG}"
  process_template "${SCRIPTDIR}/changelog.template" "${DEB_CHANGELOG}"

  # Add a line so debchange can update the timestamp and have one changelog
  # line, then use sed to add a summary of the changes since the last version
  # bump.
  debchange -a --nomultimaint -m --changelog "${DEB_CHANGELOG}" \
            "New release."
  if [ -f "${BUILDDIR}/installer/changes.txt" ]; then
      local formatted_changes=`awk '{printf "    - %s\\\\\n",$0}' ${BUILDDIR}/installer/changes.txt`
      sed -i '/New release/{n;c\
'"${formatted_changes}"'

}' ${DEB_CHANGELOG}
  fi

  GZLOG="${STAGEDIR}/usr/share/doc/${PACKAGE}/changelog.Debian.gz"
  mkdir -p "$(dirname "${GZLOG}")"
  gzip -9 -c "${DEB_CHANGELOG}" > "${GZLOG}"
  chmod 644 "${GZLOG}"
}

# Create the Debian control file needed by dpkg-deb.
gen_control() {
  dpkg-gencontrol -v"${VERSIONFULL}" -c"${DEB_CONTROL}" -l"${DEB_CHANGELOG}" \
  -f"${DEB_FILES}" -p"${PACKAGE}" -P"${STAGEDIR}" \
  -O > "${STAGEDIR}/DEBIAN/control"
  rm -f "${DEB_CONTROL}"
}

# Setup the installation directory hierachy in the package staging area.
prep_staging_debian() {
  prep_staging_common
  install -m 755 -d "${STAGEDIR}/DEBIAN" \
    "${STAGEDIR}/usr/share/doc/${PACKAGE}"
}

# Put the package contents in the staging area.
stage_install_debian() {
  local USR_BIN_SYMLINK_NAME="xwalk"

  prep_staging_debian
  stage_install_common
}

# Actually generate the package file.
do_package() {
  echo "Packaging ${ARCHITECTURE}..."

  # Need a dummy debian/control file for dpkg-shlibdeps.
  DUMMY_STAGING_DIR="${TMPFILEDIR}/dummy_staging"
  mkdir "$DUMMY_STAGING_DIR"
  pushd "$DUMMY_STAGING_DIR"
  mkdir debian
  touch debian/control
  # Generate the dependencies,
  # TODO(mmoss): This is a workaround for a problem where dpkg-shlibdeps was
  # resolving deps using some of our build output shlibs (i.e.
  # out/Release/lib.target/libfreetype.so.6), and was then failing with:
  #   dpkg-shlibdeps: error: no dependency information found for ...
  # It's not clear if we ever want to look in LD_LIBRARY_PATH to resolve deps,
  # but it seems that we don't currently, so this is the most expediant fix.
  SAVE_LDLP=${LD_LIBRARY_PATH:-}
  unset LD_LIBRARY_PATH
  DPKG_SHLIB_DEPS=$(dpkg-shlibdeps -O "${STAGEDIR}/${INSTALLDIR}/${PROGNAME}" | \
                           sed 's/^shlibs:Depends=//')
  if [ -n "$SAVE_LDLP" ]; then
      LD_LIBRARY_PATH=$SAVE_LDLP
  fi
  rm -rf "$DUMMY_STAGING_DIR"
  popd

  # Additional dependencies not in the dpkg-shlibdeps output.
  # - Pull a more recent version of NSS than required by runtime linking, for
  #   security and stability updates in NSS.
  ADDITION_DEPS="ca-certificates, libnss3 (>= 3.14.3), lsb-base (>=3.2), \
    xdg-utils (>= 1.0.2)"

  # Fix-up libnspr dependency due to renaming in Ubuntu (the old package still
  # exists, but it was moved to "universe" repository, which isn't installed by
  # default).
  DPKG_SHLIB_DEPS=$(sed \
                        's/\(libnspr4-0d ([^)]*)\), /\1 | libnspr4 (>= 4.9.5-0ubuntu0), /g' \
                        <<< $DPKG_SHLIB_DEPS)

  COMMON_DEPS="${DPKG_SHLIB_DEPS}, ${ADDITION_DEPS}"
  COMMON_PREDEPS="dpkg (>= 1.14.0)"

  PREDEPENDS="$COMMON_PREDEPS"
  DEPENDS="${COMMON_DEPS}"
  REPLACES=""
  CONFLICTS=""
  PROVIDES=""
  gen_changelog
  process_template "${SCRIPTDIR}/control.template" "${DEB_CONTROL}"
  export DEB_HOST_ARCH="${ARCHITECTURE}"
  if [ -f "${DEB_CONTROL}" ]; then
    gen_control
  fi
  fakeroot dpkg-deb -Zxz -z9 -b "${STAGEDIR}" .
}

# Remove temporary files and unwanted packaging output.
cleanup() {
  echo "Cleaning..."
  rm -rf "${STAGEDIR}"
  rm -rf "${TMPFILEDIR}"
}

usage() {
  echo "usage: $(basename $0) [-a target_arch] [-o 'dir'] "
  echo "                      [-b 'dir']"
  echo "-a arch    package architecture (ia32 or x64)"
  echo "-o dir     package output directory [${OUTPUTDIR}]"
  echo "-b dir     build input directory    [${BUILDDIR}]"
  echo "-h         this help message"
}

process_opts() {
  while getopts ":o:b:c:a:h" OPTNAME
  do
    case $OPTNAME in
      o )
        OUTPUTDIR=$(readlink -f "${OPTARG}")
        mkdir -p "${OUTPUTDIR}"
        ;;
      b )
        BUILDDIR=$(readlink -f "${OPTARG}")
        ;;
      a )
        TARGETARCH="$OPTARG"
        ;;
      h )
        usage
        exit 0
        ;;
      \: )
        echo "'-$OPTARG' needs an argument."
        usage
        exit 1
        ;;
      * )
        echo "invalid command-line option: $OPTARG"
        usage
        exit 1
        ;;
    esac
  done
}

#=========
# MAIN
#=========

SCRIPTDIR=$(readlink -f "$(dirname "$0")")
OUTPUTDIR="${PWD}"
STAGEDIR=$(mktemp -d -t deb.build.XXXXXX) || exit 1
TMPFILEDIR=$(mktemp -d -t deb.tmp.XXXXXX) || exit 1
DEB_CHANGELOG="${TMPFILEDIR}/changelog"
DEB_FILES="${TMPFILEDIR}/files"
DEB_CONTROL="${TMPFILEDIR}/control"
# Default target architecture to same as build host.
if [ "$(uname -m)" = "x86_64" ]; then
  TARGETARCH="x64"
else
  TARGETARCH="ia32"
fi

# call cleanup() on exit
trap cleanup 0
process_opts "$@"
BUILDDIR=${BUILDDIR:=$(readlink -f "${SCRIPTDIR}/../../../../out/Release")}

source ${BUILDDIR}/installer/common/installer.include

get_version_info
VERSIONFULL="${VERSION}-${PACKAGE_RELEASE}"

source ${BUILDDIR}/installer/common/crosswalk.info

# Some Debian packaging tools want these set.
export DEBFULLNAME="${MAINTNAME}"
export DEBEMAIL="${MAINTMAIL}"

# Make everything happen in the OUTPUTDIR.
cd "${OUTPUTDIR}"

case "$TARGETARCH" in
  ia32 )
    export ARCHITECTURE="i386"
    stage_install_debian
    ;;
  x64 )
    export ARCHITECTURE="amd64"
    stage_install_debian
    ;;
  * )
    echo
    echo "ERROR: Don't know how to build DEBs for '$TARGETARCH'."
    echo
    exit 1
    ;;
esac

do_package
