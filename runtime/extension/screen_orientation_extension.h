// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_EXTENSION_SCREEN_ORIENTATION_EXTENSION_H_
#define XWALK_RUNTIME_EXTENSION_SCREEN_ORIENTATION_EXTENSION_H_

#include "base/memory/scoped_ptr.h"
#include "base/values.h"

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/tizen/sensor_provider.h"

namespace xwalk {

class Runtime;

namespace extensions {
class XWalkExtensionServer;
}

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;
using extensions::XWalkExtensionServer;

enum ScreenOrientation {
  PORTRAIT_PRIMARY    = 0x1,
  PORTRAIT_SECONDARY  = 0x2,
  PORTRAIT            = 0x3,
  LANDSCAPE_PRIMARY   = 0x4,
  LANDSCAPE_SECONDARY = 0x8,
  LANDSCAPE           = 0xc,
  ANY_ORIENTATION     = 0xf,
};

class ScreenOrientationExtension : public XWalkExtension {
 public:
  explicit ScreenOrientationExtension(XWalkExtensionServer* server);
  virtual ~ScreenOrientationExtension();

 private:
  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;
};

class ScreenOrientationInstance : public XWalkExtensionInstance,
                                  public SensorProvider::Observer {
 public:
  explicit ScreenOrientationInstance(Runtime* runtime);
  virtual ~ScreenOrientationInstance();

 private:
  // XWalkExtensionInstance implementation.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  // SensorProvider::Observer implementation.
  virtual void OnRotationChanged(gfx::Display::Rotation rotation) OVERRIDE;

  void OnLock(scoped_ptr<XWalkExtensionFunctionInfo> info);

  Runtime* runtime_;
  XWalkExtensionFunctionHandler handler_;

  ScreenOrientation allowed_orientations_;
  ScreenOrientation device_orientation_;
  ScreenOrientation screen_orientation_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_EXTENSION_SCREEN_ORIENTATION_EXTENSION_H_
