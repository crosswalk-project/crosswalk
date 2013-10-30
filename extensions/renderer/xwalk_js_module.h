// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_JS_MODULE_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_JS_MODULE_H_

#include <string>
#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace xwalk {
namespace extensions {

scoped_ptr<XWalkNativeModule> CreateJSModuleFromResource(int resource_id);

// Provides a module for the Module System based on a JavaScript code. This is
// useful for providing JS helpers and small libraries to extensions creators.
//
// The JS code of a native module is executed with an object "exports" that
// should be filled with functions and properties that the module will export.
class XWalkJSModule : public XWalkNativeModule {
 public:
  explicit XWalkJSModule(const std::string& js_code);
  virtual ~XWalkJSModule();

 private:
  // XWalkNativeModule implementation.
  virtual v8::Handle<v8::Object> NewInstance() OVERRIDE;

  bool Compile(std::string* error);

  std::string js_code_;
  v8::Persistent<v8::Script> compiled_script_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_JS_MODULE_H_
