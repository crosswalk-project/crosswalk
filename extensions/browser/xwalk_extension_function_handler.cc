// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

namespace xwalk {
namespace extensions {

XWalkExtensionFunctionHandler::XWalkExtensionFunctionHandler() {}

XWalkExtensionFunctionHandler::~XWalkExtensionFunctionHandler() {}

bool XWalkExtensionFunctionHandler::HandleFunction(
    const FunctionInfo& info) {
  FunctionHandlerMap::iterator iter = handlers_.find(info.name);
  if (iter == handlers_.end())
    return false;

  iter->second.Run(info);

  return true;
}

}  // namespace extensions
}  // namespace xwalk
