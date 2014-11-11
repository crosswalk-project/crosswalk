// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_DEFERED_UI_STRATEGY_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_DEFERED_UI_STRATEGY_H_

#include <map>

#include "xwalk/runtime/browser/runtime_ui_strategy.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"

namespace xwalk {
class Runtime;

class RuntimeDeferedUIStrategy : public RuntimeUIStrategy {
 public:
  RuntimeDeferedUIStrategy();
  virtual ~RuntimeDeferedUIStrategy();

  // Override from RuntimeUIStrategy.
  virtual void Show(Runtime* runtime,
                    const NativeAppWindow::CreateParams& params) OVERRIDE;

  void ShowStoredRuntimes();

 private:
  std::map<Runtime*, NativeAppWindow::CreateParams> runtime_map_;
  bool defered_show_;
};

inline RuntimeDeferedUIStrategy*
ToRuntimeDeferedUIStrategy(RuntimeUIStrategy* ui_strategy) {
  return static_cast<RuntimeDeferedUIStrategy*>(ui_strategy);
}
}  // namespace xwalk
#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_DEFERED_UI_STRATEGY_H_
