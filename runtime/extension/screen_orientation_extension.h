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

namespace application {
class Application;
}

class Runtime;
class NativeAppWindowTizen;

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

// NOTE : This class will be removed (and content::ScreenOrientationProvider
// used instead).
class ScreenOrientationExtension : public XWalkExtension {
 public:
  explicit ScreenOrientationExtension(
      application::Application* app, OrientationMask ua_default);
  virtual ~ScreenOrientationExtension();

 private:
  // XWalkExtension overrides:
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

  application::Application* application_;
};

class ScreenOrientationInstance : public XWalkExtensionInstance,
                                  public MultiOrientationScreen::Observer {
 public:
  explicit ScreenOrientationInstance(application::Application* app);
  virtual ~ScreenOrientationInstance();

 private:
  // XWalkExtensionInstance overrides:
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  // MultiOrientationScreen::Observer overrides:
  virtual void OnOrientationChanged(Orientation orientation) OVERRIDE;

  void OnAllowedOrientationsChanged(
      scoped_ptr<XWalkExtensionFunctionInfo> info);

  XWalkExtensionFunctionHandler handler_;
  MultiOrientationScreen* screen_;

  application::Application* application_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_EXTENSION_SCREEN_ORIENTATION_EXTENSION_H_
