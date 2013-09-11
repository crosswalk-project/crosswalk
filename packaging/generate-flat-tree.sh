#!/bin/sh

# generate-flat-tree.sh
#
# Creates a tarball containing all directories (including other repositories
# like Blink and V8) of a given chromium+xwalk tree, similar to
# tools/export_tarball in the Chromium tree (which could also be used, but it's
# slower and always calls xz(1)).
#
# For more usage information, please see
# https://github.com/otcshare/crosswalk/wiki/Build-Crosswalk#building-an-rpm-package-for-tizen

if [ -z "${XWALK_PREFIX}" ]; then
    echo "You need to set XWALK_PREFIX to the top-level directory of your source tree (the one that contains src/)."
    exit 1
fi

# Ignore directories outside src/.
if [ "${GCLIENT_DEP_PATH#src}" = "${GCLIENT_DEP_PATH}" ]; then
    exit
fi

cd ${XWALK_PREFIX}
echo "Adding ${GCLIENT_DEP_PATH}..."

tar --update --file ${XWALK_PREFIX}/flat-xwalk-tree.tar \
    --exclude-vcs --exclude=native_client --exclude=LayoutTests \
    --exclude=src/out --verbose \
    ${GCLIENT_DEP_PATH}
