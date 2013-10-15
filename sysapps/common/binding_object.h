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

// This class is used to represent an object living in the JavaScript
// context that has a counterpart in the native side. The lifecycle of
// this object is controlled by the BindingObjectStore. The JavaScript
// object represented by this class must use the BindingObjectPrototype.
class BindingObject {
 public:
  // We don't assign an Instance to the handler here because this class
  // won't be receiving raw messages, but only FunctionInfo with the reply
  // callback already set to the appropriated Instance.
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
