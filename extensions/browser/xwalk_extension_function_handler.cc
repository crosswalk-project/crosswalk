// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

#include "xwalk/extensions/common/xwalk_external_instance.h"

namespace {

using xwalk::extensions::XWalkExtensionInstance;

void DispatchResult(XWalkExtensionInstance* instance,
    const std::string& callback_id, scoped_ptr<base::ListValue> result) {
  DCHECK(result);
  DCHECK(instance);

  if (callback_id.empty()) {
    DLOG(WARNING) << "Sending a reply with an empty callback id has no"
        "practical effect. This code can be optimized by not creating "
        "and not posting the result.";
    return;
  }

  // Prepend the callback id to the list, so the handlers
  // on the JavaScript side know which callback should be evoked.
  result->Insert(0, new base::StringValue(callback_id));
  instance->PostMessageToJS(result.PassAs<base::Value>());
}

}  // namespace

namespace xwalk {
namespace extensions {

XWalkExtensionFunctionHandler::XWalkExtensionFunctionHandler() {}

XWalkExtensionFunctionHandler::~XWalkExtensionFunctionHandler() {}

void XWalkExtensionFunctionHandler::HandleMessage(
    scoped_ptr<base::Value> msg, XWalkExtensionInstance* instance) {
  base::ListValue* args;
  if (!msg->GetAsList(&args) || args->GetSize() < 2) {
    // FIXME(tmpsantos): This warning could be better if the Context had a
    // pointer to the Extension. We could tell what extension sent the
    // invalid message.
    LOG(WARNING) << "Invalid number of arguments.";
    return;
  }

  // The first parameter stands for the function signature.
  std::string function_name;
  if (!args->GetString(0, &function_name)) {
    LOG(WARNING) << "The function name is not a string.";
    return;
  }

  // The second parameter stands for callback id, the remaining
  // ones are the function arguments.
  std::string callback_id;
  if (!args->GetString(1, &callback_id)) {
    LOG(WARNING) << "The callback id is not a string.";
    return;
  }

  // We reuse args to pass the extra arguments to the handler, so remove
  // function_name and callback_id from it.
  args->Remove(0, NULL);
  args->Remove(0, NULL);

  XWalkExtensionFunctionInfo info;
  info.name = function_name;
  info.arguments = args;
  info.post_result_cb = base::Bind(&DispatchResult, instance, callback_id);

  if (!HandleFunction(info)) {
    DLOG(WARNING) << "Function not registered: " << function_name;
    return;
  }
}

bool XWalkExtensionFunctionHandler::HandleFunction(
    const XWalkExtensionFunctionInfo& info) {
  FunctionHandlerMap::iterator iter = handlers_.find(info.name);
  if (iter == handlers_.end())
    return false;

  iter->second.Run(info);

  return true;
}

}  // namespace extensions
}  // namespace xwalk
