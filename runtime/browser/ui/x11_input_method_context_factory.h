// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_X11_INPUT_METHOD_CONTEXT_FACTORY_H_
#define XWALK_RUNTIME_BROWSER_UI_X11_INPUT_METHOD_CONTEXT_FACTORY_H_

#include "ui/base/ime/linux/linux_input_method_context_factory.h"

namespace xwalk {

class X11InputMethodContextFactory : public ui::LinuxInputMethodContextFactory {
 public:
  X11InputMethodContextFactory();
  ~X11InputMethodContextFactory() override;

  // LinuxInputMethodContextFactory implementation.
  scoped_ptr<ui::LinuxInputMethodContext> CreateInputMethodContext(
      ui::LinuxInputMethodContextDelegate* delegate) const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(X11InputMethodContextFactory);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_X11_INPUT_METHOD_CONTEXT_FACTORY_H_
