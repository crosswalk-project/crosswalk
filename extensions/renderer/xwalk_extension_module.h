// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MODULE_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MODULE_H_

#include <string>
#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace WebKit {
class WebFrame;
}

namespace base {
class Value;
}

namespace content {
class V8ValueConverter;
}

namespace xwalk {
namespace extensions {

// Responsible for running the JS code of a XWalkExtension. This includes
// creating and exposing an 'extension' object for the execution context of
// the extension JS code.
//
// We'll create one XWalkExtensionModule per extension/frame pair, so
// there'll be a set of different modules per v8::Context.
class XWalkExtensionModule {
 public:
  XWalkExtensionModule(v8::Handle<v8::Context> context,
                       const std::string& extension_name,
                       const std::string& extension_code);
  virtual ~XWalkExtensionModule();

  void DispatchMessageToListener(v8::Handle<v8::Context> context,
                                 const base::Value& msg);

  std::string extension_name() const { return extension_name_; }

 private:
  void LoadExtensionCode(v8::Handle<v8::Context> context);

  void SetFunction(const char* name, v8::InvocationCallback callback);

  // Callbacks for JS functions available in 'extension' object.
  static v8::Handle<v8::Value> PostMessageCallback(const v8::Arguments& args);
  static v8::Handle<v8::Value> SendSyncMessageCallback(
      const v8::Arguments& args);
  static v8::Handle<v8::Value> SetMessageListenerCallback(
      const v8::Arguments& args);

  static XWalkExtensionModule* GetExtensionModuleFromArgs(
      const v8::Arguments& args);

  // Template for the 'extension' object exposed to the extension JS code.
  v8::Persistent<v8::ObjectTemplate> object_template_;

  // This JS object contains a pointer back to the XWalkExtensionModule, it is
  // set as data for the function callbacks.
  v8::Persistent<v8::Object> function_data_;

  // Function to be called when the extension sends a message to its JS code.
  // This value is registered by using 'extension.setMessageListener()'.
  v8::Persistent<v8::Function> message_listener_;

  std::string extension_name_;
  std::string extension_code_;

  // TODO(cmarcelo): Move to a single converter, since we always use same
  // parameters.
  scoped_ptr<content::V8ValueConverter> converter_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MODULE_H_
