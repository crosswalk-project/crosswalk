// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_defered_ui_strategy.h"

#include "xwalk/runtime/browser/runtime.h"

namespace xwalk {
RuntimeDeferedUIStrategy::RuntimeDeferedUIStrategy()
    : defered_show_(true) {}

RuntimeDeferedUIStrategy::~RuntimeDeferedUIStrategy() {}

void RuntimeDeferedUIStrategy::Show(
    Runtime* runtime, const NativeAppWindow::CreateParams& params) {
  if (!defered_show_) {
    RuntimeUIStrategy::Show(runtime, params);
    return;
  }

  runtime_map_[runtime] = params;
}

void RuntimeDeferedUIStrategy::ShowStoredRuntimes() {
  for (const auto& item : runtime_map_) {
    RuntimeUIStrategy::Show(item.first, item.second);
  }
  defered_show_ = false;
}
}  // namespace xwalk
