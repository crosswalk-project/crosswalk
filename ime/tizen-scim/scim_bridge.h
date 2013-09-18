// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/ime/text_input_type.h"

#ifndef UI_BASE_IME_TIZEN_SCIM_BRIDGE_H_
#define UI_BASE_IME_TIZEN_SCIM_BRIDGE_H_

namespace ui {

class SCIMBridgeImpl;

class SCIMBridge {

public:
  SCIMBridge();
  ~SCIMBridge();

  void Init();
  void SetFocusedWindow(unsigned long);
  void TextInputChanged(ui::TextInputType);

private:
  scoped_ptr<SCIMBridgeImpl> impl_;
  DISALLOW_COPY_AND_ASSIGN(SCIMBridge);
};

}

#endif  // UI_BASE_IME_TIZEN_SCIM_BRIDGE_H_

