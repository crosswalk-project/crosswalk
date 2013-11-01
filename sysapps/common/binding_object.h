// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_COMMON_BINDING_OBJECT_H_
#define XWALK_SYSAPPS_COMMON_BINDING_OBJECT_H_

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

namespace xwalk {
namespace sysapps {

using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;

// This class is used to represent every object living in the JavaScript
// context that has a counterpart in the native side. The lifecycle of
// this object is controlled by the BindingObjectStore.
class BindingObject {
 public:
  BindingObject() : handler_(NULL) {}
  virtual ~BindingObject() {}

  bool HandleFunction(scoped_ptr<XWalkExtensionFunctionInfo> info) {
    return handler_.HandleFunction(info.Pass());
  }

 protected:
  XWalkExtensionFunctionHandler handler_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_COMMON_BINDING_OBJECT_H_
