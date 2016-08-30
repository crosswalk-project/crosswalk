#!/bin/bash
# Copyright (c) 2015 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Generates a list of changes since the last version revision commit bump for
# later consumption by, for example, the packaging system for Linux distros.

set -e

if [ $# -ne 1 ]; then
    echo "Usage: $1 <output file>"
    exit 2
fi

SCRIPTDIR=$(readlink -f "$(dirname "$0")")
REPOROOT=$(readlink -f "${SCRIPTDIR}/../../..")

if [ ! -d ${REPOROOT}/.git ]; then
    echo "Not in a git checkout. Not generating commit log for packaging."
    exit 0
fi

export GIT_DIR=${REPOROOT}/.git

last_bump_rev=`git log -1 --author='Crosswalk Release Engineering' --format='%H'`
if [ -z "${last_bump_rev}" ]; then
    echo "It looks like there has never been a version bump commit before."
    echo "Are you in src/xwalk?"
    exit 1
fi

OUTPUT=`readlink -f $1`
git log --no-merges --reverse --format='%s' $last_bump_rev.. > $OUTPUT
