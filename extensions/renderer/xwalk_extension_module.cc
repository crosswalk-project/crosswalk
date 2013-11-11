// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_module.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "content/public/renderer/v8_value_converter.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"
#include "xwalk/extensions/renderer/xwalk_v8_utils.h"

namespace xwalk {
namespace extensions {

namespace {

// This is the key used in the data object passed to our callbacks to store a
// pointer back to XWalkExtensionModule.
const char* kXWalkExtensionModule = "kXWalkExtensionModule";

}  // namespace

XWalkExtensionModule::XWalkExtensionModule(XWalkExtensionClient* client,
                                           XWalkModuleSystem* module_system,
                                           const std::string& extension_name,
                                           const std::string& extension_code)
    : extension_name_(extension_name),
      extension_code_(extension_code),
      converter_(content::V8ValueConverter::create()),
      client_(client),
      module_system_(module_system),
      instance_id_(0) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
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

  function_data_.Reset(isolate, function_data);
  object_template_.Reset(isolate, object_template);
}

XWalkExtensionModule::~XWalkExtensionModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  // Deleting the data will disable the functions, they'll return early. We do
  // this because it might be the case that the JS objects we created outlive
  // this object, even if we destroy the references we have.
  // TODO(cmarcelo): Add a test for this case.
  // FIXME(cmarcelo): These calls are causing crashes on shutdown with Chromium
  //                  29.0.1547.57 and had to be commented out.
  // v8::Handle<v8::Object> function_data =
  //     v8::Handle<v8::Object>::New(isolate, function_data_);
  // function_data->Delete(v8::String::New(kXWalkExtensionModule));

  object_template_.Dispose();
  object_template_.Clear();
  function_data_.Dispose();
  function_data_.Clear();
  message_listener_.Dispose();
  message_listener_.Clear();

  if (instance_id_)
    client_->DestroyInstance(instance_id_);
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
  // We take care here to make sure that line numbering for api_code after
  // wrapping doesn't change, so that syntax errors point to the correct line.
  return base::StringPrintf(
      "var %s; (function(extension, requireNative) { "
      "extension.internal = {};"
      "extension.internal.sendSyncMessage = extension.sendSyncMessage;"
      "delete extension.sendSyncMessage;"
      "return (function(exports) {'use strict'; %s\n})(%s); });",
      CodeToEnsureNamespace(extension_name).c_str(),
      extension_code.c_str(),
      extension_name.c_str());
}

v8::Handle<v8::Value> RunString(const std::string& code,
                                std::string* exception) {
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Handle<v8::String> v8_code(v8::String::New(code.c_str()));

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::TryCatch try_catch;
  try_catch.SetVerbose(true);

  v8::Handle<v8::Script> script(v8::Script::New(v8_code, v8::String::Empty()));
  if (try_catch.HasCaught()) {
    *exception = ExceptionToString(try_catch);
    return v8::Undefined();
  }

  v8::Handle<v8::Value> result = script->Run();
  if (try_catch.HasCaught()) {
    *exception = ExceptionToString(try_catch);
    return v8::Undefined();
  }

  return handle_scope.Close(result);
}

}  // namespace

void XWalkExtensionModule::LoadExtensionCode(
    v8::Handle<v8::Context> context, v8::Handle<v8::Function> requireNative) {
  CHECK(!instance_id_);
  instance_id_ = client_->CreateInstance(extension_name_, this);

  std::string exception;
  std::string wrapped_api_code = WrapAPICode(extension_code_, extension_name_);
  v8::Handle<v8::Value> result =
      RunString(wrapped_api_code, &exception);
  if (!result->IsFunction()) {
    LOG(WARNING) << "Couldn't load JS API code for " << extension_name_
      << ": " << exception;
    return;
  }
  v8::Handle<v8::Function> callable_api_code =
      v8::Handle<v8::Function>::Cast(result);
  v8::Handle<v8::ObjectTemplate> object_template =
      v8::Handle<v8::ObjectTemplate>::New(context->GetIsolate(),
                                          object_template_);

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
        << extension_name_ << ": " << ExceptionToString(try_catch);
  }
}

void XWalkExtensionModule::HandleMessageFromNative(const base::Value& msg) {
  if (message_listener_.IsEmpty())
    return;

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Context> context = module_system_->GetV8Context();
  v8::Context::Scope context_scope(context);

  v8::Handle<v8::Value> v8_value(converter_->ToV8Value(&msg, context));
  v8::Handle<v8::Function> message_listener =
      v8::Handle<v8::Function>::New(isolate, message_listener_);;

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::TryCatch try_catch;
  message_listener->Call(context->Global(), 1, &v8_value);
  if (try_catch.HasCaught())
    LOG(WARNING) << "Exception when running message listener: "
        << ExceptionToString(try_catch);
}

// static
void XWalkExtensionModule::PostMessageCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::ReturnValue<v8::Value> result(info.GetReturnValue());
  XWalkExtensionModule* module = GetExtensionModule(info);
  if (!module || info.Length() != 1) {
    result.Set(false);
    return;
  }

  v8::Handle<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  scoped_ptr<base::Value> value(
      module->converter_->FromV8Value(info[0], context));

  CHECK(module->instance_id_);
  module->client_->PostMessageToNative(module->instance_id_, value.Pass());
  result.Set(true);
}

// static
void XWalkExtensionModule::SendSyncMessageCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::ReturnValue<v8::Value> result(info.GetReturnValue());
  XWalkExtensionModule* module = GetExtensionModule(info);
  if (!module || info.Length() != 1) {
    result.Set(false);
    return;
  }

  v8::Handle<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  scoped_ptr<base::Value> value(
      module->converter_->FromV8Value(info[0], context));

  CHECK(module->instance_id_);
  scoped_ptr<base::Value> reply(
      module->client_->SendSyncMessageToNative(module->instance_id_,
                                               value.Pass()));

  // If we tried to send a message to an instance that became invalid,
  // then reply will be NULL.
  if (reply)
    result.Set(module->converter_->ToV8Value(reply.get(), context));
}

// static
void XWalkExtensionModule::SetMessageListenerCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::ReturnValue<v8::Value> result(info.GetReturnValue());
  XWalkExtensionModule* module = GetExtensionModule(info);
  if (!module || info.Length() != 1) {
    result.Set(false);
    return;
  }

  if (!info[0]->IsFunction() && !info[0]->IsUndefined()) {
    LOG(WARNING) << "Trying to set message listener with invalid value.";
    result.Set(false);
    return;
  }

  v8::Isolate* isolate = info.GetIsolate();
  if (info[0]->IsUndefined()) {
    module->message_listener_.Dispose();
    module->message_listener_.Clear();
  } else {
    module->message_listener_.Dispose();
    module->message_listener_.Reset(isolate, info[0].As<v8::Function>());
  }

  result.Set(true);
}

// static
XWalkExtensionModule* XWalkExtensionModule::GetExtensionModule(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::HandleScope handle_scope(info.GetIsolate());
  v8::Local<v8::Object> data = info.Data().As<v8::Object>();
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
