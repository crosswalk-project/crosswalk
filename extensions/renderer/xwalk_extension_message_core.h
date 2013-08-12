// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MESSAGE_CORE_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MESSAGE_CORE_H_

#include <map>
#include <string>
#include "base/values.h"
#include "v8/include/v8.h"

namespace content {
class V8ValueConverter;
}

namespace xwalk {
namespace extensions {
class V8Context;

// Extension infra's message passing core object. It will be created by renderer
// controller, the wrapper object will be passed to API JS binding function as
// an argument. OnPostMessage() handles function callback, the wrapper object
// will be retrieved from a persistent object handle.
class XWalkExtensionMessageCore {
 public:
  XWalkExtensionMessageCore();
  ~XWalkExtensionMessageCore();

  v8::Handle<v8::Object> GetMessageCoreWrapper(
    V8Context* v8_context, const std::string& extension_name);
  void OnWillReleaseScriptContext(V8Context* v8_context);
  void OnPostMessage(
      V8Context* v8_context,
      const std::string& extension_name,
      const base::ListValue& msg);

 private:
  static v8::Handle<v8::Value> PostMessage(const v8::Arguments& args);
  static v8::Handle<v8::Value> SendSyncMessage(const v8::Arguments& args);
  static scoped_ptr<base::ListValue> V8ValueToListValue(
      const v8::Handle<v8::Value> value);
  static v8::Handle<v8::Value> RequireNative(const v8::Arguments& args);

  static content::V8ValueConverter* converter_;

  v8::Handle<v8::Object> CreateMessageCore(V8Context* v8_context);
  v8::Handle<v8::Object> CreateMessageCoreWrapper(
    V8Context* v8_context,
    const std::string& extension_name,
    v8::Handle<v8::Object> message_core);

  typedef std::map<V8Context*, v8::Persistent<v8::Object> > MessageCoreMap;
  MessageCoreMap message_core_map_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionMessageCore);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_MESSAGE_CORE_H_
