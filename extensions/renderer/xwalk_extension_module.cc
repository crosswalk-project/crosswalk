// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_module.h"

#include "base/logging.h"
#include "base/stringprintf.h"
#include "base/values.h"
#include "content/public/renderer/v8_value_converter.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScopedMicrotaskSuppression.h"
#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"

namespace xwalk {
namespace extensions {

namespace {

// This is the key used in the data object passed to our callbacks to store a
// pointer back to XWalkExtensionModule.
const char* kXWalkExtensionModule = "kXWalkExtensionModule";

}  // namespace

XWalkExtensionModule::XWalkExtensionModule(
    v8::Handle<v8::Context> context,
    const std::string& extension_name,
    const std::string& extension_code)
    : extension_name_(extension_name),
      extension_code_(extension_code),
      converter_(content::V8ValueConverter::create()) {

  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Object> function_data = v8::Object::New();
  function_data->Set(v8::String::New(kXWalkExtensionModule),
                     v8::External::New(this));

  v8::Handle<v8::ObjectTemplate> object_template = v8::ObjectTemplate::New();
  object_template->Set(
      "postMessage",
      v8::FunctionTemplate::New(PostMessageCallback, function_data));
  object_template->Set(
      "sendSyncMessage",
      v8::FunctionTemplate::New(SendSyncMessageCallback, function_data));
  object_template->Set(
      "setMessageListener",
      v8::FunctionTemplate::New(SetMessageListenerCallback, function_data));

  function_data_ = v8::Persistent<v8::Object>::New(isolate, function_data);
  object_template_= v8::Persistent<v8::ObjectTemplate>::New(
      isolate, object_template);
}

XWalkExtensionModule::~XWalkExtensionModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  // Deleting the data will disable the functions, they'll return early. We do
  // this because it might be the case that the JS objects we created outlive
  // this object, even if we destroy the references we have.
  // TODO(cmarcelo): Add a test for this case.
  v8::Handle<v8::Object> function_data = function_data_;
  function_data->Delete(v8::String::New(kXWalkExtensionModule));

  object_template_.Dispose(isolate);
  object_template_.Clear();
  function_data_.Dispose(isolate);
  function_data_.Clear();
  message_listener_.Dispose(isolate);
  message_listener_.Clear();
}

void XWalkExtensionModule::DispatchMessageToListener(
    v8::Handle<v8::Context> context, const base::Value& msg) {
  if (message_listener_.IsEmpty())
    return;

  v8::Isolate* isolate = context->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(context);

  v8::Handle<v8::Value> v8_value(converter_->ToV8Value(&msg, context));
  v8::Handle<v8::Function> message_listener = message_listener_;

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::TryCatch try_catch;
  message_listener->Call(context->Global(), 1, &v8_value);
  if (try_catch.HasCaught())
    LOG(WARNING) << "Exception when running message listener";
}

namespace {

std::string CodeToEnsureNamespace(const std::string& extension_name) {
  std::string result;
  size_t pos = 0;
  while (true) {
    pos = extension_name.find('.', pos);
    if (pos == std::string::npos) {
      result += extension_name + " = {};";
      break;
    }
    std::string ns = extension_name.substr(0, pos);
    result += ns + " = " + ns + " || {}; ";
    pos++;
  }
  return result;
}

// Wrap API code into a callable form that takes extension object as parameter.
std::string WrapAPICode(const std::string& extension_code,
                        const std::string& extension_name) {
  // FIXME(cmarcelo): For now sync messaging is disabled on Windows because we
  // jump through the UI process and this is not supported in that platform. See
  // issue https://github.com/otcshare/crosswalk/issues/268 for details.

  // We take care here to make sure that line numbering for api_code after
  // wrapping doesn't change, so that syntax errors point to the correct line.
  return base::StringPrintf(
      "var %s; (function(extension, requireNative) { "
      "extension._setupExtensionInternal = function() {"
      "  xwalk._setupExtensionInternal(extension);"
      "};"
#if !defined(OS_WIN)
      "extension.internal = {};"
      "extension.internal.sendSyncMessage = extension.sendSyncMessage;"
#endif
      "delete extension.sendSyncMessage;"
      "return (function(exports) {'use strict'; %s\n})(%s); });",
      CodeToEnsureNamespace(extension_name).c_str(),
      extension_code.c_str(),
      extension_name.c_str());
}

v8::Handle<v8::Value> RunString(const std::string& code,
                                const std::string& name) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::String> v8_code(v8::String::New(code.c_str()));
  v8::Handle<v8::String> v8_name(v8::String::New(name.c_str()));

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::TryCatch try_catch;
  try_catch.SetVerbose(true);

  v8::Handle<v8::Script> script(v8::Script::New(v8_code, v8_name));
  if (try_catch.HasCaught())
    return v8::Undefined();

  v8::Handle<v8::Value> result = script->Run();
  if (try_catch.HasCaught())
    return v8::Undefined();

  return handle_scope.Close(result);
}

}  // namespace

void XWalkExtensionModule::LoadExtensionCode(
    v8::Handle<v8::Context> context, v8::Handle<v8::Function> requireNative) {
  std::string wrapped_api_code = WrapAPICode(extension_code_, extension_name_);
  v8::Handle<v8::Value> result =
      RunString(wrapped_api_code, "JS API code for " + extension_name_);
  if (!result->IsFunction()) {
    LOG(WARNING) << "Couldn't load JS API code for " << extension_name_;
    return;
  }
  v8::Handle<v8::Function> callable_api_code =
      v8::Handle<v8::Function>::Cast(result);
  v8::Handle<v8::ObjectTemplate> object_template = object_template_;

  const int argc = 2;
  v8::Handle<v8::Value> argv[argc] = {
    object_template->NewInstance(),
    requireNative
  };

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::TryCatch try_catch;
  try_catch.SetVerbose(true);
  callable_api_code->Call(context->Global(), argc, argv);
  if (try_catch.HasCaught()) {
    LOG(WARNING) << "Exception while loading JS API code for "
                 << extension_name_;
  }
}

// static
v8::Handle<v8::Value> XWalkExtensionModule::PostMessageCallback(
    const v8::Arguments& args) {
  XWalkExtensionModule* module = GetExtensionModuleFromArgs(args);
  if (!module)
    return v8::False();

  if (args.Length() != 1)
    return v8::False();

  v8::Handle<v8::Context> context = args.GetIsolate()->GetCurrentContext();
  scoped_ptr<base::Value> value(
      module->converter_->FromV8Value(args[0], context));

  WebKit::WebFrame* frame = WebKit::WebFrame::frameForContext(context);
  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForFrame(frame);

  if (!handler->PostMessageToExtension(
          frame->identifier(), module->extension_name_, value.Pass()))
    return v8::False();
  return v8::True();
}

// static
v8::Handle<v8::Value> XWalkExtensionModule::SendSyncMessageCallback(
    const v8::Arguments& args) {
  XWalkExtensionModule* module = GetExtensionModuleFromArgs(args);
  if (!module)
    return v8::False();

  if (args.Length() != 1)
    return v8::False();

  v8::Handle<v8::Context> context = args.GetIsolate()->GetCurrentContext();
  scoped_ptr<base::Value> value(
      module->converter_->FromV8Value(args[0], context));

  WebKit::WebFrame* frame = WebKit::WebFrame::frameForContext(context);
  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForFrame(frame);

  scoped_ptr<base::Value> reply(handler->SendSyncMessageToExtension(
      frame->identifier(), module->extension_name_, value.Pass()));

  return module->converter_->ToV8Value(reply.get(), context);
}

// static
v8::Handle<v8::Value> XWalkExtensionModule::SetMessageListenerCallback(
    const v8::Arguments& args) {
  XWalkExtensionModule* module = GetExtensionModuleFromArgs(args);
  if (!module)
    return v8::False();

  if (args.Length() != 1)
    return v8::False();

  if (!args[0]->IsFunction() && !args[0]->IsUndefined()) {
    LOG(WARNING) << "Trying to set message listener with invalid value.";
    return v8::False();
  }

  v8::Isolate* isolate = args.GetIsolate();
  if (args[0]->IsUndefined()) {
    module->message_listener_.Dispose(isolate);
    module->message_listener_.Clear();
  } else {
    module->message_listener_.Dispose(isolate);
    module->message_listener_ =
        v8::Persistent<v8::Function>::New(isolate, args[0].As<v8::Function>());
  }

  return v8::True();
}

// static
XWalkExtensionModule* XWalkExtensionModule::GetExtensionModuleFromArgs(
    const v8::Arguments& args) {
  v8::HandleScope handle_scope(args.GetIsolate());
  v8::Local<v8::Object> data = args.Data().As<v8::Object>();
  v8::Local<v8::Value> module =
      data->Get(v8::String::New(kXWalkExtensionModule));
  if (module.IsEmpty() || module->IsUndefined()) {
    LOG(WARNING) << "Trying to use extension from already destroyed context!";
    return NULL;
  }
  CHECK(module->IsExternal());
  return static_cast<XWalkExtensionModule*>(module.As<v8::External>()->Value());
}

}  // namespace extensions
}  // namespace xwalk
