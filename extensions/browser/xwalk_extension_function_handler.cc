// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

#include "base/location.h"
#include "xwalk/extensions/common/xwalk_external_instance.h"

namespace xwalk {
namespace extensions {

XWalkExtensionFunctionInfo::XWalkExtensionFunctionInfo(
    const std::string& name,
    scoped_ptr<base::ListValue> arguments,
    const PostResultCallback& post_result_cb)
  : name_(name),
    arguments_(arguments.Pass()),
    post_result_cb_(post_result_cb) {}

XWalkExtensionFunctionInfo::~XWalkExtensionFunctionInfo() {}

XWalkExtensionFunctionHandler::XWalkExtensionFunctionHandler(
    XWalkExtensionInstance* instance)
  : instance_(instance),
    weak_factory_(this) {}

XWalkExtensionFunctionHandler::~XWalkExtensionFunctionHandler() {}

void XWalkExtensionFunctionHandler::HandleMessage(scoped_ptr<base::Value> msg) {
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

  scoped_ptr<XWalkExtensionFunctionInfo> info(
      new XWalkExtensionFunctionInfo(
          function_name,
          make_scoped_ptr(static_cast<base::ListValue*>(msg.release())),
          base::Bind(&XWalkExtensionFunctionHandler::DispatchResult,
                     weak_factory_.GetWeakPtr(),
                     base::MessageLoopProxy::current(),
                     callback_id)));

  if (!HandleFunction(info.Pass())) {
    DLOG(WARNING) << "Function not registered: " << function_name;
    return;
  }
}

bool XWalkExtensionFunctionHandler::HandleFunction(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  FunctionHandlerMap::iterator iter = handlers_.find(info->name());
  if (iter == handlers_.end())
    return false;

  iter->second.Run(info.Pass());

  return true;
}

// static
void XWalkExtensionFunctionHandler::DispatchResult(
    const base::WeakPtr<XWalkExtensionFunctionHandler>& handler,
    scoped_refptr<base::MessageLoopProxy> client_task_runner,
    const std::string& callback_id,
    scoped_ptr<base::ListValue> result) {
  DCHECK(result);

  if (client_task_runner != base::MessageLoopProxy::current()) {
    client_task_runner->PostTask(FROM_HERE,
        base::Bind(&XWalkExtensionFunctionHandler::DispatchResult,
                   handler,
                   client_task_runner,
                   callback_id,
                   base::Passed(&result)));
    return;
  }

  if (callback_id.empty()) {
    DLOG(WARNING) << "Sending a reply with an empty callback id has no"
        "practical effect. This code can be optimized by not creating "
        "and not posting the result.";
    return;
  }

  // Prepend the callback id to the list, so the handlers
  // on the JavaScript side know which callback should be evoked.
  result->Insert(0, new base::StringValue(callback_id));

  if (handler)
    handler->PostMessageToInstance(result.PassAs<base::Value>());
}

void XWalkExtensionFunctionHandler::PostMessageToInstance(
    scoped_ptr<base::Value> msg) {
  instance_->PostMessageToJS(msg.Pass());
}

}  // namespace extensions
}  // namespace xwalk
