// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/renderer/application_native_module.h"

#include "base/logging.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/v8_value_converter.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebView.h"

namespace xwalk {
namespace application {

namespace {

// This is the key used in the data object passed to our callbacks to store a
// pointer back to ApplicationNativeModule.
const char* kApplicationNativeModule = "kApplicationNativeModule";

ApplicationNativeModule* GetNativeModule(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::HandleScope handle_scope(info.GetIsolate());
  v8::Handle<v8::Object> data = info.Data().As<v8::Object>();
  v8::Handle<v8::Value> module_value =
    data->Get(v8::String::New(kApplicationNativeModule));
  CHECK(*module_value && module_value->IsExternal());
  ApplicationNativeModule* module = static_cast<ApplicationNativeModule*>(
      module_value.As<v8::External>()->Value());
  return module;
}

}  // namespace

void ApplicationNativeModule::GetViewByIDCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().SetUndefined();
  if (info.Length() != 1 || !info[0]->IsInt32())
    return;

  int main_routing_id = info[0]->ToInt32()->Value();
  content::RenderView* render_view =
    content::RenderView::FromRoutingID(main_routing_id);
  if (!render_view)
    return;

  WebKit::WebView* webview = render_view->GetWebView();
  v8::Handle<v8::Context> context =
    webview->mainFrame()->mainWorldScriptContext();
  v8::Handle<v8::Value> window = context->Global();
  info.GetReturnValue().Set(window);
}

ApplicationNativeModule::ApplicationNativeModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Object> function_data = v8::Object::New();
  function_data->Set(
      v8::String::New(kApplicationNativeModule), v8::External::New(this));

  // Register native function templates to object template here.
  v8::Handle<v8::ObjectTemplate> object_template = v8::ObjectTemplate::New();
  object_template->Set(
      "getViewByID",
      v8::FunctionTemplate::New(&ApplicationNativeModule::GetViewByIDCallback,
                                function_data));

  function_data_.Reset(isolate, function_data);
  object_template_.Reset(isolate, object_template);
}

ApplicationNativeModule::~ApplicationNativeModule() {
  object_template_.Dispose();
  object_template_.Clear();
  function_data_.Dispose();
  function_data_.Clear();
}

v8::Handle<v8::Object> ApplicationNativeModule::NewInstance() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template =
      v8::Handle<v8::ObjectTemplate>::New(isolate, object_template_);
  return handle_scope.Close(object_template->NewInstance());
}

}  // namespace application
}  // namespace xwalk
