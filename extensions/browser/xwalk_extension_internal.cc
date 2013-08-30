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

XWalkInternalExtension::InternalContext::~InternalContext() {
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

  FunctionHandler* handler = GetHandlerForFunction(function_name);
  if (!handler) {
    DLOG(WARNING) << "Function not registered: " << function_name;
    return;
  }

  // We reuse args to pass the extra arguments to the handler, so remove
  // function_name and callback_id from it.
  args->Remove(0, NULL);
  args->Remove(0, NULL);
  handler->Run(function_name, callback_id, args);
}

scoped_ptr<base::Value>
XWalkInternalExtension::InternalContext::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  HandleMessage(msg.Pass());

  DCHECK(sync_result_) << "Result not set for synchronous function call.";
  return sync_result_.Pass();
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

void XWalkInternalExtension::InternalContext::PostResultSync(
    scoped_ptr<base::Value> result) {
  // Overwriting a previously set result is fatal because it means
  // we are calling this function twice in the same handler.
  DCHECK(result && !sync_result_);

  sync_result_ = result.Pass();
}

}  // namespace extensions
}  // namespace xwalk
