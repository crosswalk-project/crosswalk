// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_
#define XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_

#include <string>
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class RuntimeExtension : public XWalkExtension {
 public:
  RuntimeExtension();

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;
};

class RuntimeInstance : public XWalkExtensionInstance {
 public:
  explicit RuntimeInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  void OnGetAPIVersion(scoped_ptr<XWalkExtensionFunctionInfo> info);

  XWalkExtensionFunctionHandler handler_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_
