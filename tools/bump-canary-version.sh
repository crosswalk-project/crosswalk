#!/bin/sh

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script bumps Crosswalk's build version in all the places we need.
# Quick recap: Crosswalk's version follows the "MAJOR.MINOR.BUILD.PATCH"
# scheme. Each canary (aka nightly) is supposed to monotonically increase the
# BUILD number.
# It is supposed to be run from within the Crosswalk directory tree, and will
# fail if run from outside.

ROOT=`git rev-parse --show-toplevel 2>/dev/null`
if [ $? -ne 0 ]; then
    echo "This script is supposed to be run from the Crosswalk directory tree."
    exit 2
fi

# VERSION
TMPFILE=`mktemp`
perl -pe '/BUILD=(\d+)/ && s/($1)/$1+1/e' ${ROOT}/VERSION > ${TMPFILE}
mv ${TMPFILE} ${ROOT}/VERSION
echo "Updated VERSION"

# packaging/crosswalk.spec
TMPFILE=`mktemp`
perl -pe '/Version: +(\d+\.){2}(\d+)/ && s/($2)/$1+1/e' \
     ${ROOT}/packaging/crosswalk.spec > ${TMPFILE}
mv ${TMPFILE} ${ROOT}/packaging/crosswalk.spec
echo "Updated packaging/crosswalk.spec"
