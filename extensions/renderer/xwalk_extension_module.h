// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MODULE_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MODULE_H_

#include <string>
#include "xwalk/extensions/renderer/xwalk_extension_client.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace WebKit {
class WebFrame;
}

namespace content {
class V8ValueConverter;
}

namespace xwalk {
namespace extensions {

class XWalkExtensionClient;
class XWalkModuleSystem;

// Responsible for running the JS code of a XWalkExtension. This includes
// creating and exposing an 'extension' object for the execution context of
// the extension JS code.
//
// We'll create one XWalkExtensionModule per extension/frame pair, so
// there'll be a set of different modules per v8::Context.
class XWalkExtensionModule : public XWalkExtensionClient::InstanceHandler {
 public:
  XWalkExtensionModule(XWalkExtensionClient* client,
                       XWalkModuleSystem* module_system,
                       const std::string& extension_name,
                       const std::string& extension_code);
  virtual ~XWalkExtensionModule();

  // TODO(cmarcelo): Make this return a v8::Handle<v8::Object>, and
  // let the module system set it to the appropriated object.
  void LoadExtensionCode(v8::Handle<v8::Context> context,
                         v8::Handle<v8::Function> requireNative);

  std::string extension_name() const { return extension_name_; }

 private:
  // XWalkExtensionClient::InstanceHandler implementation.
  virtual void HandleMessageFromNative(const base::Value& msg) OVERRIDE;

  // Callbacks for JS functions available in 'extension' object.
  static void PostMessageCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void SendSyncMessageCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void SetMessageListenerCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  static XWalkExtensionModule* GetExtensionModule(
      const v8::FunctionCallbackInfo<v8::Value>& info);

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

  XWalkExtensionClient* client_;
  XWalkModuleSystem* module_system_;
  int64_t instance_id_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MODULE_H_
