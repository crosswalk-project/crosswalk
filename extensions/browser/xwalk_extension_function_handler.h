// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_FUNCTION_HANDLER_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_FUNCTION_HANDLER_H_

#include <map>
#include <string>
#include "base/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/values.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionInstance;

// This struct is passed to the function handler, usually assigned to the
// signature of a method in JavaScript. The struct can be safely passed around.
class XWalkExtensionFunctionInfo {
 public:
  typedef base::Callback<void(scoped_ptr<base::ListValue> result)>
      PostResultCallback;

  XWalkExtensionFunctionInfo(const std::string& name,
                             scoped_ptr<base::ListValue> arguments,
                             const PostResultCallback& post_result_cb);

  ~XWalkExtensionFunctionInfo();

  // Convenience method for posting the results back to the renderer process.
  // The object identifier is already wrapped at the |post_result_cb|. This
  // will ultimately dispatch the result to the appropriated instance or
  // do nothing in case the instance doesn't exist anymore. PostResult can
  // be called from any thread.
  void PostResult(scoped_ptr<base::ListValue> result) const {
    post_result_cb_.Run(result.Pass());
  };

  std::string name() const {
    return name_;
  }

  base::ListValue* arguments() const {
    return arguments_.get();
  }

  PostResultCallback post_result_cb() const {
    return post_result_cb_;
  }

 private:
  std::string name_;
  scoped_ptr<base::ListValue> arguments_;

  PostResultCallback post_result_cb_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionFunctionInfo);
};

// Helper for handling JavaScript method calls in the native side. Allows you to
// register a handler for a function with a given signature. This class takes an
// XWalkExtensionInstance in the constructor and should never outlive this
// instance.
class XWalkExtensionFunctionHandler {
 public:
  typedef base::Callback<void(
      scoped_ptr<XWalkExtensionFunctionInfo> info)> FunctionHandler;

  explicit XWalkExtensionFunctionHandler(XWalkExtensionInstance* instance);
  ~XWalkExtensionFunctionHandler();

  // Converts a raw message from the renderer to a XWalkExtensionFunctionInfo
  // data structure and invokes HandleFunction().
  void HandleMessage(scoped_ptr<base::Value> msg);

  // Executes the handler associated to the |name| tag of the |info| argument
  // passed as parameter.
  bool HandleFunction(scoped_ptr<XWalkExtensionFunctionInfo> info);

  // This method will register a callback to handle a message tagged as
  // |function_name|. When invoked, the handler will get a
  // XWalkExtensionFunctionInfo struct with the function |name| (which can be
  // used in case a handler is in charge of more than one function). |arguments|
  // contains the list of parameters ready to be used as input for
  // Params::Create() generated from the IDL description of the API. The reply,
  // if necessary, should be posted back using the PostResult() method of the
  // XWalkExtensionFunctionInfo provided.
  //
  // The signature of a function handler should be like the following:
  //
  //   void Foobar::OnShow(scoped_ptr<XWalkExtensionFunctionInfo> info);
  //
  // And register them like this, preferable at the Foobar constructor:
  //
  //   Register("show", base::Bind(&Foobar::OnShow, base::Unretained(this)));
  //   Register("getStuff", base::Bind(&Foobar::OnGetStuff)); // Static method.
  //   ...
  void Register(const std::string& function_name, FunctionHandler callback) {
    handlers_[function_name] = callback;
  }

 private:
  static void DispatchResult(
      const base::WeakPtr<XWalkExtensionFunctionHandler>& handler,
      scoped_refptr<base::MessageLoopProxy> client_task_runner,
      const std::string& callback_id,
      scoped_ptr<base::ListValue> result);

  void PostMessageToInstance(scoped_ptr<base::Value> msg);

  typedef std::map<std::string, FunctionHandler> FunctionHandlerMap;
  FunctionHandlerMap handlers_;

  XWalkExtensionInstance* instance_;
  base::WeakPtrFactory<XWalkExtensionFunctionHandler> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionFunctionHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_FUNCTION_HANDLER_H_
