#!/bin/bash
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SCRIPT_DIR="$(dirname "${BASH_SOURCE:-$0}")"

. ${SCRIPT_DIR}/../../../build/android/envsetup.sh "$@"

unset CHROMIUM_GYP_FILE

export PATH=$PATH:${CHROME_SRC}/xwalk/build/android

xwalk_android_gyp() {
  echo "GYP_GENERATORS set to '$GYP_GENERATORS'"
  (
    "${CHROME_SRC}/xwalk/gyp_xwalk" --check "$@"
  )
}
