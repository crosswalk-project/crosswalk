// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_message_core.h"

#include "base/stringprintf.h"
#include "base/values.h"
#include "content/public/renderer/v8_value_converter.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScopedMicrotaskSuppression.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"
#include "xwalk/extensions/renderer/xwalk_extension_v8_context.h"

// This will be generated from xwalk_api.js.
extern const char kSource_xwalk_api[];

namespace xwalk {
namespace extensions {

content::V8ValueConverter* XWalkExtensionMessageCore::converter_ = NULL;

XWalkExtensionMessageCore::XWalkExtensionMessageCore() {
  DCHECK(!converter_);
  converter_ = content::V8ValueConverter::create();
}

XWalkExtensionMessageCore::~XWalkExtensionMessageCore() {
  delete converter_;
}

v8::Handle<v8::Object> XWalkExtensionMessageCore::GetMessageCoreWrapper(
    V8Context* v8_context, const std::string& extension_name) {
  v8::Handle<v8::Object> message_core;
  MessageCoreMap::iterator it = message_core_map_.find(v8_context);
  if (it != message_core_map_.end()) {
    message_core = it->second;
  } else {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    message_core = CreateMessageCore(v8_context);
    message_core_map_[v8_context] = v8::Persistent<v8::Object>(
        isolate, message_core);
  }

  return CreateMessageCoreWrapper(v8_context, extension_name, message_core);
}

v8::Handle<v8::Object> XWalkExtensionMessageCore::CreateMessageCore(
    V8Context* v8_context) {
  v8::Handle<v8::Context> context = v8_context->context();
  v8::Handle<v8::Object> global = context->Global();

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::Handle<v8::Value> message_core_func = v8::Script::Compile(
      v8::String::New(kSource_xwalk_api))->Run();
  v8::Handle<v8::Function> requireNativeHandle = v8::FunctionTemplate::New(
      RequireNative)->GetFunction();
  v8::Handle<v8::Value> message_core_args[] = { requireNativeHandle };
  v8::Handle<v8::Value> message_core =
    message_core_func.As<v8::Function>()->Call(global, 1, message_core_args);

  return message_core.As<v8::Object>();
}

v8::Handle<v8::Object> XWalkExtensionMessageCore::CreateMessageCoreWrapper(
    V8Context* v8_context,
    const std::string& extension_name,
    v8::Handle<v8::Object> message_core) {
  const char* name = extension_name.c_str();
  std::string message_core_wrapper_str = base::StringPrintf(
    "(function() {"
    "  return function (messageCore) {"
    "    return {"
    "      postMessage: function(msg) { messageCore.postMessage('%s', msg); },"
    "      setMessageListener: function(listener) {"
    "                       messageCore.setMessageListener('%s', listener); },"
    "      onpostmessage: function(msg) {"
    "                       messageCore.onpostmessage('%s', msg); }"
#if !defined(OS_WIN)
    "    , internal: {"
    "        sendSyncMessage: function(msg) {"
    "          return messageCore.sendSyncMessage('%s', msg); }"
    "      }"
#endif
    "    }; };"
    "})();",
    name, name, name
#if !defined(OS_WIN)
    , name
#endif
  ); // NOLINT

  v8::Handle<v8::Context> context = v8_context->context();
  v8::Handle<v8::Object> global = context->Global();

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::Handle<v8::Value> wrapper_func = v8::Script::Compile(
      v8::String::New(message_core_wrapper_str.c_str()))->Run();
  v8::Handle<v8::Value> wrapper_args[] = { message_core.As<v8::Object>() };
  v8::Handle<v8::Value> message_core_wrapper =
    wrapper_func.As<v8::Function>()->Call(global, 1, wrapper_args);

  return message_core_wrapper.As<v8::Object>();
}

void XWalkExtensionMessageCore::OnWillReleaseScriptContext(
    V8Context* v8_context) {
  MessageCoreMap::iterator it = message_core_map_.find(v8_context);
  DCHECK(it != message_core_map_.end());

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  (it->second).Dispose(isolate);
  message_core_map_.erase(it);
}

void XWalkExtensionMessageCore::OnPostMessage(
    V8Context* v8_context,
    const std::string& extension_name,
    const base::ListValue& msg) {
  v8::HandleScope handle_scope;
  MessageCoreMap::iterator it = message_core_map_.find(v8_context);
  CHECK(it != message_core_map_.end());
  v8::Handle<v8::Object> message_core = it->second;

  v8::Handle<v8::Context> context(v8_context->context());
  v8::Context::Scope context_scope(context);

  // We get the message wrapped in a ListValue because Value doesn't have
  // param traits.
  const base::Value* value;
  msg.Get(0, &value);

  scoped_ptr<content::V8ValueConverter> converter(
      content::V8ValueConverter::create());
  v8::Handle<v8::Value> v8_value(
      converter->ToV8Value(value, context));
  const int argc = 2;
  v8::Handle<v8::Value> argv[argc] = {
    v8::String::New(extension_name.c_str()),
    v8_value
  };
  v8::Handle<v8::Value> callback =
    message_core->Get(v8::String::New("onpostmessage"));

  // Note: see comment in WebScopedMicrotaskSuppression.h to understand why we
  // are not using V8 API directly but going through frame.
  WebKit::WebFrame* frame = v8_context->frame();
  frame->callFunctionEvenIfScriptDisabled(callback.As<v8::Function>(),
                                          context->Global(),
                                          argc, argv);
}

v8::Handle<v8::Value> XWalkExtensionMessageCore::PostMessage(
    const v8::Arguments& args) {
  if (args.Length() != 2)
    return v8::False();

  scoped_ptr<base::ListValue> list_value_args(V8ValueToListValue(args[1]));
  if (!list_value_args)
    return v8::False();

  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForCurrentContext();

  std::string extension(*v8::String::Utf8Value(args[0]));
  if (!handler->PostMessageToExtension(extension, *list_value_args))
    return v8::False();
  return v8::True();
}

v8::Handle<v8::Value> XWalkExtensionMessageCore::SendSyncMessage(
    const v8::Arguments& args) {
  if (args.Length() != 2)
    return v8::False();

  scoped_ptr<base::ListValue> list_value_args(V8ValueToListValue(args[1]));
  if (!list_value_args)
    return v8::False();

  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForCurrentContext();

  std::string extension(*v8::String::Utf8Value(args[0]));
  scoped_ptr<base::ListValue> reply(handler->SendSyncMessageToExtension(
          extension, *list_value_args));

  const base::Value* value;
  reply->Get(0, &value);

  return converter_->ToV8Value(value, handler->GetContext());
}

scoped_ptr<base::ListValue> XWalkExtensionMessageCore::V8ValueToListValue(
      const v8::Handle<v8::Value> v8_value) {
  // We first convert a V8 Value to a base::Value and later we wrap it
  // into a base::ListValue for dispatching to the browser process. We need
  // this wrapping because base::Value doesn't have param traits and
  // implementing one is not a viable option (would require fork base::Value
  // and create a new empty type).
  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForCurrentContext();

  scoped_ptr<base::Value> value(
      converter_->FromV8Value(v8_value, handler->GetContext()));

  if (!value)
    return scoped_ptr<base::ListValue>(NULL);

  scoped_ptr<base::ListValue> list_value(new base::ListValue);
  list_value->Append(value.release());

  return list_value.Pass();
}

v8::Handle<v8::Value> XWalkExtensionMessageCore::RequireNative(
    const v8::Arguments& args) {
  v8::String::Utf8Value required_name(args[0]);

  if (!strcmp(*required_name, "PostMessage")) {
    v8::Handle<v8::Function> func = v8::FunctionTemplate::New(
        &PostMessage)->GetFunction();
    return func;
  } else if (!strcmp(*required_name, "SendSyncMessage")) {
    v8::Handle<v8::Function> func = v8::FunctionTemplate::New(
        &SendSyncMessage)->GetFunction();
    return func;
  }

  return v8::Undefined();
}

}  // namespace extensions
}  // namespace xwalk
