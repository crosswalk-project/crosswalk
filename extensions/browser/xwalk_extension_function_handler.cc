// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

namespace xwalk {
namespace extensions {

XWalkExtensionFunctionHandler::XWalkExtensionFunctionHandler() {}

XWalkExtensionFunctionHandler::~XWalkExtensionFunctionHandler() {}

bool XWalkExtensionFunctionHandler::HandleFunction(std::string& function_name,
    const std::string& callback_id, base::ListValue* args) {
  FunctionHandlerMap::iterator iter = handlers_.find(function_name);
  if (iter == handlers_.end())
    return false;

  iter->second.Run(function_name, callback_id, args);

  return true;
}

}  // namespace extensions
}  // namespace xwalk
