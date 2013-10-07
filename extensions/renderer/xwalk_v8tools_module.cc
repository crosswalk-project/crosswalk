// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

#include "base/logging.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "xwalk/extensions/renderer/xwalk_v8_utils.h"

namespace xwalk {
namespace extensions {

namespace {

void ForceSetPropertyCallback(const v8::FunctionCallbackInfo<v8::Value>& info) {
  // FIXME(cmarcelo): is this the default?
  info.GetReturnValue().SetUndefined();

  if (info.Length() != 3 || !info[0]->IsObject() || !info[1]->IsString()) {
    return;
  }
  info[0].As<v8::Object>()->ForceSet(info[1], info[2]);
}

void LifecycleTrackerCleanup(v8::Isolate* isolate,
                             v8::Persistent<v8::Object>* tracker,
                             void*) {
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> local_tracker =
      v8::Local<v8::Object>::New(isolate, *tracker);
  v8::Handle<v8::Value> function =
      local_tracker->Get(v8::String::New("destructor"));

  if (function.IsEmpty() || !function->IsFunction()) {
    DLOG(WARNING) << "Destructor function not set for LifecycleTracker.";
    tracker->Dispose();
    return;
  }

  v8::Handle<v8::Context> context = v8::Context::New(isolate);
  WebKit::WebScopedMicrotaskSuppression suppression;

  v8::TryCatch try_catch;
  v8::Handle<v8::Function>::Cast(function)->Call(context->Global(), 0, NULL);
  if (try_catch.HasCaught())
    LOG(WARNING) << "Exception when running LifecycleTracker destructor: "
        << ExceptionToString(try_catch);

  tracker->Dispose();
}

void LifecycleTracker(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Persistent<v8::Object> tracker(isolate, v8::Object::New());
  tracker.MakeWeak<void>(NULL, &LifecycleTrackerCleanup);

  info.GetReturnValue().Set(tracker);
}

}  // namespace

XWalkV8ToolsModule::XWalkV8ToolsModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template = v8::ObjectTemplate::New();
  object_template->Set("forceSetProperty",
                        v8::FunctionTemplate::New(ForceSetPropertyCallback));
  object_template->Set("lifecycleTracker",
                       v8::FunctionTemplate::New(LifecycleTracker));

  object_template_.Reset(isolate, object_template);
}

XWalkV8ToolsModule::~XWalkV8ToolsModule() {
  object_template_.Dispose();
  object_template_.Clear();
}

v8::Handle<v8::Object> XWalkV8ToolsModule::NewInstance() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template =
      v8::Handle<v8::ObjectTemplate>::New(isolate, object_template_);
  return handle_scope.Close(object_template->NewInstance());
}

}  // namespace extensions
}  // namespace xwalk
