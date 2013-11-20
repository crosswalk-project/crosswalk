#!/bin/sh

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script is expected to be run by `git-buildpackage' (called by `gbs
# build') as a postexport hook. Its purpose is to remove the tar file generated
# by `gbs export' with `git archive' and create a new archive with tar itself.
#
# It is helpful in two ways:
# - It automates the generation of a proper archive for RPM builds so that one
#   does not need to create a separate git tree with all sources for `gbs
#   build' to work.
# - Since tar is used, the archive's members all have their actual mtimes as
#   opposed to the time of the git tree-ish passed to `git archive'. This is
#   part of the solution for incremental builds in Tizen: since we use actual
#   file mtimes, they are not rebuilt by `make'.
#
# As a postexport hook, there are two additional environment variables
# available: GBP_GIT_DIR is /path/to/src/xwalk/.git, and GBP_TMP_DIR is the
# temporary directory containing everything in /path/to/src/xwalk/packaging
# that will be copied to $GBSROOT/local/sources and used by `rpmbuild' to build
# an RPM package.
#
# The script is run from GBP_TMP_DIR.

# Fail early to avoid bigger problems later in the process.
set -e

TAR_FILE="${GBP_TMP_DIR}/crosswalk.tar"

if [ ! -f "${TAR_FILE}" ]; then
    echo "${TAR_FILE} does not exist. Aborting."
    exit 1
fi

# The top-level directory that _contains_ src/.
BASE_SRC_DIR=`readlink -f "${GBP_GIT_DIR}/../../.."`
if [ $? -ne 0 ]; then
    echo "${GBP_GIT_DIR}/../../.. does not seem to be a valid path. Aborting."
    exit 1
fi

# Erase the archive generated with `git archive'.
rm -v "${TAR_FILE}"

echo "Creating a new ${TAR_FILE} from ${BASE_SRC_DIR}/src"

# The --transform parameter is used to prepend all archive members with
# crosswalk/ so they all share a common root. Note it is crosswalk/, without
# any version numbers, so that any build files in an external directory
# referring to a source file does not need to be updated just because of a
# version bump.
tar --update --file "${TAR_FILE}" \
    --exclude-vcs --exclude=native_client --exclude=LayoutTests \
    --exclude=src/out --directory="${BASE_SRC_DIR}" \
    --transform="s:^:crosswalk/:S" \
    src
