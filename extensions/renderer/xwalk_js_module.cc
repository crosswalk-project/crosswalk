// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_js_module.h"

#include "base/logging.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "xwalk/extensions/renderer/xwalk_v8_utils.h"

namespace xwalk {
namespace extensions {

XWalkJSModule::XWalkJSModule(const std::string& js_code)
    : js_code_(js_code) {
}

XWalkJSModule::~XWalkJSModule() {
  compiled_script_.Reset();
}

v8::Handle<v8::Object> XWalkJSModule::NewInstance() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  if (compiled_script_.IsEmpty()) {
    std::string compilation_error;
    if (!Compile(&compilation_error)) {
      LOG(WARNING) << "Error compiling JS module: " << compilation_error;
      return v8::Handle<v8::Object>();
    }
  }

  v8::Handle<v8::Script> script =
      v8::Handle<v8::Script>::New(isolate, compiled_script_);

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::TryCatch try_catch;
  v8::Handle<v8::Value> result = script->Run();
  if (try_catch.HasCaught()) {
    LOG(WARNING) << "Error during requireNative(): "
                 << ExceptionToString(try_catch);
    return v8::Handle<v8::Object>();
  }

  return handle_scope.Close(result.As<v8::Object>());
}

bool XWalkJSModule::Compile(std::string* error) {
  std::string wrapped_js_code =
      "'use strict'; (function() { var exports = {}; (function(exports) {"
      + js_code_ + "})(exports); return exports; })()";

  v8::Handle<v8::String> v8_code(v8::String::New(wrapped_js_code.c_str()));

  WebKit::WebScopedMicrotaskSuppression suppression;
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script(v8::Script::New(v8_code, v8::String::Empty()));
  if (try_catch.HasCaught()) {
    *error = "Error compiling JS module: " + ExceptionToString(try_catch);
    return false;
  }

  compiled_script_.Reset(v8::Isolate::GetCurrent(), script);
  return true;
}

}  // namespace extensions
}  // namespace xwalk
