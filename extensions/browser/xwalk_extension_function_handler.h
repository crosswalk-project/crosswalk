// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_FUNCTION_HANDLER_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_FUNCTION_HANDLER_H_

#include <map>
#include <string>
#include "base/bind.h"
#include "base/values.h"

namespace xwalk {
namespace extensions {

struct XWalkExtensionFunctionInfo {
  std::string name;
  std::string callback_id;
  base::ListValue* arguments;
};

// Helper for handling JavaScript method calls in the native side. Allows you to
// register a handler for a function with a given signature.
class XWalkExtensionFunctionHandler {
 public:
  typedef base::Callback<void(
      const XWalkExtensionFunctionInfo& info)> FunctionHandler;

  XWalkExtensionFunctionHandler();
  ~XWalkExtensionFunctionHandler();

  bool HandleFunction(const XWalkExtensionFunctionInfo& info);

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
  //   void Foobar::OnShow(const XWalkExtensionFunctionInfo& info);
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
  typedef std::map<std::string, FunctionHandler> FunctionHandlerMap;
  FunctionHandlerMap handlers_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionFunctionHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_FUNCTION_HANDLER_H_
