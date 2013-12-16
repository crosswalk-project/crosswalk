// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_EXTENSION_SCREEN_ORIENTATION_EXTENSION_H_
#define XWALK_RUNTIME_EXTENSION_SCREEN_ORIENTATION_EXTENSION_H_

#include "base/memory/scoped_ptr.h"
#include "base/values.h"

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/ui/screen_orientation.h"

namespace xwalk {

class Runtime;
class NativeAppWindowTizen;

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class ScreenOrientationExtension : public XWalkExtension {
 public:
  explicit ScreenOrientationExtension();
  virtual ~ScreenOrientationExtension();

 private:
  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;
};

class ScreenOrientationInstance : public XWalkExtensionInstance,
                                  public ScreenOrientation::Observer {
 public:
  explicit ScreenOrientationInstance(Runtime* runtime);
  virtual ~ScreenOrientationInstance();

 private:
  // XWalkExtensionInstance implementation.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  // ScreenOrientation::Observer implementation.
  virtual void OnScreenOrientationChanged(
      ScreenOrientation::Mode mode) OVERRIDE;

  void OnLock(scoped_ptr<XWalkExtensionFunctionInfo> info);

  NativeAppWindowTizen* window_;
  XWalkExtensionFunctionHandler handler_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_EXTENSION_SCREEN_ORIENTATION_EXTENSION_H_
