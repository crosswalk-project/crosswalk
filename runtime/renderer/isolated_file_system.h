// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_ISOLATED_FILE_SYSTEM_H_
#define XWALK_RUNTIME_RENDERER_ISOLATED_FILE_SYSTEM_H_

#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace xwalk {
namespace extensions {

class XWalkModuleSystem;

class IsolatedFileSystem: public xwalk::extensions::XWalkNativeModule {
 public:
  IsolatedFileSystem();
  virtual ~IsolatedFileSystem();

 private:
  virtual v8::Handle<v8::Object> NewInstance() OVERRIDE;
  static void GetIsolatedFileSystem(const v8::FunctionCallbackInfo<v8::Value>&);

  v8::Persistent<v8::ObjectTemplate> object_template_;
  v8::Persistent<v8::Object> function_data_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_ISOLATED_FILE_SYSTEM_H_
