// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_internal.h"

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"

namespace xwalk {
namespace extensions {

XWalkInternalExtension::Context* XWalkInternalExtension::CreateContext(
    const XWalkExtension::PostMessageCallback& post_message) {
  return new InternalContext(post_message);
}

XWalkInternalExtension::InternalContext::InternalContext(
    const XWalkExtension::PostMessageCallback& post_message)
  : XWalkExtension::Context(post_message) {
}

XWalkInternalExtension::InternalContext::FunctionHandler*
    XWalkInternalExtension::InternalContext::GetHandlerForFunction(
    const std::string& function) {
  FunctionHandlerMap::iterator iter = handlers_.find(function);
  if (iter == handlers_.end())
    return NULL;

  return &iter->second;
}

void XWalkInternalExtension::InternalContext::HandleMessage(
    scoped_ptr<base::Value> msg) {
  base::ListValue* args;
  if (!msg->GetAsList(&args) || args->GetSize() == 0) {
    // FIXME(tmpsantos): This warning could be better if the Context had a
    // pointer to the Extension. We could tell what extension sent the
    // invalid message.
    LOG(WARNING) << "The arguments are not in a list or the list is empty.";
    return;
  }

  // The first parameter stands for the function signature.
  base::Value* function_name;
  args->Remove(0, &function_name);
  scoped_ptr<base::Value> function_name_ref(function_name);

  if (!function_name->IsType(base::Value::TYPE_STRING)) {
    LOG(WARNING) << "The function name is not a string.";
    return;
  }

  std::string function_name_str;
  function_name->GetAsString(&function_name_str);

  // The second parameter stands for callback id, the remaining
  // ones are the function arguments.
  base::Value* callback_id;
  args->Remove(0, &callback_id);
  scoped_ptr<base::Value> callback_id_ref(callback_id);

  if (!callback_id->IsType(base::Value::TYPE_STRING)) {
    LOG(WARNING) << "The callback id is not a string.";
    return;
  }

  std::string callback_id_str;
  callback_id->GetAsString(&callback_id_str);

  FunctionHandler* handler = GetHandlerForFunction(function_name_str);
  if (!handler) {
    DLOG(WARNING) << "Function not registered: " << function_name_str;
    return;
  }

  handler->Run(function_name_str, callback_id_str, args);
}

void XWalkInternalExtension::InternalContext::PostResult(
    const std::string& callback_id, scoped_ptr<base::ListValue> result) {
  DCHECK(result);

  if (callback_id.empty()) {
    DLOG(WARNING) << "Sending a reply with an empty callback id has no"
        "practical effect. This code can be optimized by not creating "
        "and not posting the result.";
    return;
  }

  // Prepend the callback id to the list, so the handlers
  // on the JavaScript side know which callback should be evoked.
  result->Insert(0, new base::StringValue(callback_id));
  PostMessage(scoped_ptr<base::Value>(result.release()));
}

}  // namespace extensions
}  // namespace xwalk
