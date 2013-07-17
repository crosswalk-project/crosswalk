#!/bin/bash
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SCRIPT_DIR="$(dirname "${BASH_SOURCE:-$0}")"

. ${SCRIPT_DIR}/../../../build/android/envsetup.sh "$@"

# sdk_build_init() sets GYP_GENERATOR_FLAGS to 'All' target and
# CHROMIUM_GYP_FILE to all_android.gyp. Unset these so that xwalk.gyp
# is processed.
# TODO(girish): Remove only default_target=All from GYP_GENERATOR_FLAGS
unset GYP_GENERATOR_FLAGS
unset CHROMIUM_GYP_FILE

xwalk_android_gyp() {
  echo "GYP_GENERATORS set to '$GYP_GENERATORS'"
  (
    "${CHROME_SRC}/xwalk/gyp_xwalk" --check "$@"
  )
}
