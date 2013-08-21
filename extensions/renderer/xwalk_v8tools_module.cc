// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"


namespace xwalk {
namespace extensions {

namespace {

v8::Handle<v8::Value> ForceSetPropertyCallback(const v8::Arguments& args) {
  if (args.Length() != 3 || !args[0]->IsObject() || !args[1]->IsString())
    return v8::Undefined();
  args[0].As<v8::Object>()->ForceSet(args[1], args[2]);
  return v8::Undefined();
}

}  // namespace

XWalkV8ToolsModule::XWalkV8ToolsModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  object_template_ = v8::Persistent<v8::ObjectTemplate>::New(
      isolate, v8::ObjectTemplate::New());
  object_template_->Set("forceSetProperty",
                        v8::FunctionTemplate::New(ForceSetPropertyCallback));
}

XWalkV8ToolsModule::~XWalkV8ToolsModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  object_template_.Dispose(isolate);
  object_template_.Clear();
}

v8::Handle<v8::Object> XWalkV8ToolsModule::NewInstance() {
  return object_template_->NewInstance();
}

}  // namespace extensions
}  // namespace xwalk
