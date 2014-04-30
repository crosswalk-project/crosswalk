#!/bin/bash
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SCRIPT_DIR="$(dirname "${BASH_SOURCE:-$0}")"

. ${SCRIPT_DIR}/../../../build/android/envsetup.sh "$@"

export PATH=$PATH:${CHROME_SRC}/xwalk/build/android

# The purpose of this function is to do the same as android_gyp(), but calling
# gyp_xwalk instead.
xwalk_android_gyp() {
  "${CHROME_SRC}/xwalk/gyp_xwalk" --check "$@"
}
