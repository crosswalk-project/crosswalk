// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/event_types.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/ime/text_input_type.h"

#ifndef XWALK_IME_TIZEN_SCIM_SCIM_BRIDGE_X11_H_
#define XWALK_IME_TIZEN_SCIM_SCIM_BRIDGE_X11_H_

namespace ui {

class TextInputClient;
class SCIMBridgeImpl;
class InputMethodSCIM;

// This class acts as a bridge between SCIM input method framework
// and Chromium InputMethod interface.
class SCIMBridge {
 public:
  explicit SCIMBridge(InputMethodSCIM* input_method);
  ~SCIMBridge();

  void SetTextInputClient(TextInputClient* client);
  void OnTextInputChanged(ui::TextInputType type);
  void DispatchKeyEvent(const base::NativeEvent& native_event);

 private:
  scoped_ptr<SCIMBridgeImpl> impl_;
  DISALLOW_COPY_AND_ASSIGN(SCIMBridge);
};

}  // namespace ui

#endif  // XWALK_IME_TIZEN_SCIM_SCIM_BRIDGE_X11_H_

