// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_UI_STRATEGY_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_UI_STRATEGY_H_

#include "xwalk/runtime/browser/ui/native_app_window.h"

namespace xwalk {
class Runtime;

class RuntimeUIStrategy {
 public:
  RuntimeUIStrategy();
  virtual ~RuntimeUIStrategy();
  virtual void Show(Runtime* runtime,
                    const NativeAppWindow::CreateParams& params);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_UI_STRATEGY_H_
