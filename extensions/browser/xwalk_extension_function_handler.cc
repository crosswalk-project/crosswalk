// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

#include "base/location.h"
#include "base/memory/ptr_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "xwalk/extensions/common/xwalk_external_instance.h"

namespace xwalk {
namespace extensions {

XWalkExtensionFunctionInfo::XWalkExtensionFunctionInfo(
    const std::string& name,
    std::unique_ptr<base::ListValue> arguments,
    const PostResultCallback& post_result_cb)
  : name_(name),
    arguments_(std::move(arguments)),
    post_result_cb_(post_result_cb) {}

XWalkExtensionFunctionInfo::~XWalkExtensionFunctionInfo() {}

XWalkExtensionFunctionHandler::XWalkExtensionFunctionHandler(
    XWalkExtensionInstance* instance)
  : instance_(instance),
    weak_factory_(this) {}

XWalkExtensionFunctionHandler::~XWalkExtensionFunctionHandler() {}

void XWalkExtensionFunctionHandler::HandleMessage(std::unique_ptr<base::Value> msg) {
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

  std::unique_ptr<XWalkExtensionFunctionInfo> info(
      new XWalkExtensionFunctionInfo(
          function_name,
          base::WrapUnique(static_cast<base::ListValue*>(msg.release())),
          base::Bind(&XWalkExtensionFunctionHandler::DispatchResult,
                     weak_factory_.GetWeakPtr(),
                     base::ThreadTaskRunnerHandle::IsSet()
                         ? base::ThreadTaskRunnerHandle::Get()
                         : nullptr,
                     callback_id)));

  if (!HandleFunction(std::move(info))) {
    DLOG(WARNING) << "Function not registered: " << function_name;
    return;
  }
}

bool XWalkExtensionFunctionHandler::HandleFunction(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  FunctionHandlerMap::iterator iter = handlers_.find(info->name());
  if (iter == handlers_.end())
    return false;

  iter->second.Run(std::move(info));

  return true;
}

// static
void XWalkExtensionFunctionHandler::DispatchResult(
    const base::WeakPtr<XWalkExtensionFunctionHandler>& handler,
    scoped_refptr<base::SingleThreadTaskRunner> client_task_runner,
    const std::string& callback_id,
    std::unique_ptr<base::ListValue> result) {
  DCHECK(result);

  // The client_task_runner.get() call is to support using this class on a
  // thread without a message loop.
  if (client_task_runner.get() &&
      !client_task_runner->BelongsToCurrentThread()) {
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
    handler->PostMessageToInstance(std::move(result));
}

void XWalkExtensionFunctionHandler::PostMessageToInstance(
    std::unique_ptr<base::Value> msg) {
  instance_->PostMessageToJS(std::move(msg));
}

}  // namespace extensions
}  // namespace xwalk
