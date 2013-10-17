// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_RENDERER_APPLICATION_NATIVE_MODULE_H_
#define XWALK_APPLICATION_RENDERER_APPLICATION_NATIVE_MODULE_H_

#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace xwalk {
namespace application {

class ApplicationNativeModule : public xwalk::extensions::XWalkNativeModule {
 public:
  ApplicationNativeModule();
  virtual ~ApplicationNativeModule();

 private:
  virtual v8::Handle<v8::Object> NewInstance() OVERRIDE;

  // Return main frame window object according to the routing id.
  static void GetViewByIDCallback(const v8::FunctionCallbackInfo<v8::Value>&);

  v8::Persistent<v8::ObjectTemplate> object_template_;
  v8::Persistent<v8::Object> function_data_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_RENDERER_APPLICATION_NATIVE_MODULE_H_
