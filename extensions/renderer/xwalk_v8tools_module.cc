// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"


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

}  // namespace

XWalkV8ToolsModule::XWalkV8ToolsModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template = v8::ObjectTemplate::New();
  object_template->Set("forceSetProperty",
                        v8::FunctionTemplate::New(ForceSetPropertyCallback));

  object_template_.Reset(isolate, object_template);
}

XWalkV8ToolsModule::~XWalkV8ToolsModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  object_template_.Dispose(isolate);
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
