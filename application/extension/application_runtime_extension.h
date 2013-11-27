// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_EXTENSION_APPLICATION_RUNTIME_EXTENSION_H_
#define XWALK_APPLICATION_EXTENSION_APPLICATION_RUNTIME_EXTENSION_H_

#include <string>

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {

namespace application {
class ApplicationSystem;
}

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class ApplicationRuntimeExtension : public XWalkExtension {
 public:
  explicit ApplicationRuntimeExtension(
      application::ApplicationSystem* application_system);

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

 private:
  application::ApplicationSystem* application_system_;
};

class AppRuntimeExtensionInstance : public XWalkExtensionInstance {
 public:
  explicit AppRuntimeExtensionInstance(
      application::ApplicationSystem* application_system);

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  void OnGetMainDocumentID(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetManifest(scoped_ptr<XWalkExtensionFunctionInfo> info);

  application::ApplicationSystem* application_system_;

  XWalkExtensionFunctionHandler handler_;
};

}  // namespace xwalk

#endif  // XWALK_APPLICATION_EXTENSION_APPLICATION_RUNTIME_EXTENSION_H_
