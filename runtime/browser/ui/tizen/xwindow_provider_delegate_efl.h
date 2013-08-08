// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_TIZEN_XWINDOW_PROVIDER_DELEGATE_EFL_H_
#define XWALK_RUNTIME_BROWSER_UI_TIZEN_XWINDOW_PROVIDER_DELEGATE_EFL_H_

namespace views {

class XWindowProviderDelegate {
 public:
 public:
  virtual void CloseWindow() = 0;

 protected:
  virtual ~XWindowProviderDelegate() {}
};

}  // namespace views

#endif  // XWALK_RUNTIME_BROWSER_UI_TIZEN_XWINDOW_PROVIDER_DELEGATE_EFL_H_
