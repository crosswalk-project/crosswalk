// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_
#define XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_

#include <string>
#include "xwalk/extensions/browser/xwalk_extension_internal.h"

namespace xwalk {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionInstance;
using extensions::XWalkInternalExtension;
using extensions::XWalkInternalExtensionInstance;

class RuntimeExtension : public XWalkInternalExtension {
 public:
  RuntimeExtension();

  virtual const char* GetJavaScriptAPI() OVERRIDE;

  virtual XWalkExtensionInstance* CreateInstance(
      const XWalkExtension::PostMessageCallback& post_message) OVERRIDE;
};

class RuntimeInstance : public XWalkInternalExtensionInstance {
 public:
  explicit RuntimeInstance(
      const XWalkExtension::PostMessageCallback& post_message);

 private:
  void OnGetAPIVersion(const FunctionInfo& info);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_
