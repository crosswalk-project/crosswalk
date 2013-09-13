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

// Base class for every class that will handle JavaScript method calls in the
// native side. Allows you to register a handler for a function with a given
// signature.
class XWalkExtensionFunctionHandler {
 public:
  XWalkExtensionFunctionHandler();
  virtual ~XWalkExtensionFunctionHandler();

  bool HandleFunction(std::string& function_name,
                      const std::string& callback_id,
                      base::ListValue* args);

 protected:
  // This method will register a function to handle a message tagged as
  // |function_name|. When invoked, the handler will get as first parameter
  // the |function_name| (which can be used in case a handler is in charge of
  // more than one function). The |callback_id| is a unique identifier that
  // should be returned on the PostResult() in case the function triggers a
  // callback (empty string otherwise). Finally, |args| contains the list of
  // parameters ready to be used as input for Params::Create() generated from
  // the IDL description of the API.
  //
  // The signature of a function handler should be like the following:
  //
  //   void Foo::OnShowBar(const std::string& function_name,
  //                       const std::string& callback_id,
  //                       base::ListValue* args)
  //
  // And register them like this, preferable at the FooInstance constructor:
  //
  //   RegisterFunction("showBar", &FooInstance::OnShowBar);
  //   RegisterFunction("getStuff", &FooInstance::OnGetStuff);
  //   ...
  template <class T>
  void RegisterFunction(const std::string& function_name,
      void (T::*handler)(const std::string& function_name,
      const std::string& callback_id, base::ListValue* args)) {
    handlers_[function_name] = base::Bind(handler,
        base::Unretained(static_cast<T*>(this)));
  }

 private:
  typedef base::Callback<void(const std::string&, const std::string&,
                              base::ListValue*)> FunctionHandler;
  typedef std::map<std::string, FunctionHandler> FunctionHandlerMap;

  FunctionHandlerMap handlers_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionFunctionHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_FUNCTION_HANDLER_H_
