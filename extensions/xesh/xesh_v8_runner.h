// Copyright 2012 the V8 project authors. All rights reserved.
//
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_XESH_XESH_V8_RUNNER_H_
#define XWALK_EXTENSIONS_XESH_XESH_V8_RUNNER_H_

#include <string>
#include "v8/include/v8.h"

class XEShV8Runner {
 public:
  explicit XEShV8Runner(v8::Handle<v8::Context> context);
  ~XEShV8Runner() {}

  // Executes a string within the current v8 context.
  std::string ExecuteString(std::string statement);

 private:
  void RegisterAccessors(v8::Handle<v8::Context> context);

  std::string ReportException(v8::TryCatch* try_catch);

  static void PrintCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void QuitCallback(v8::Local<v8::String> property,
      const v8::PropertyCallbackInfo<v8::Value>& info);
};

#endif  // XWALK_EXTENSIONS_XESH_XESH_V8_RUNNER_H_
