// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/runtime_select_file_policy.h"

RuntimeSelectFilePolicy::RuntimeSelectFilePolicy() {}
RuntimeSelectFilePolicy::~RuntimeSelectFilePolicy() {}

bool RuntimeSelectFilePolicy::CanOpenSelectFileDialog() {
  // TODO(wang16): Implement CanOpenSelectFileDialog
  return true;
}

void RuntimeSelectFilePolicy::SelectFileDenied() {
  // TODO(wang16): Implement SelectFileDenied
}
