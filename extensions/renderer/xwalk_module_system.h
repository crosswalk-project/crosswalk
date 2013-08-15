// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_MODULE_SYSTEM_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_MODULE_SYSTEM_H_

#include <string>
#include <map>
#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionModule;

// This object is associated with the v8::Context of each WebFrame. It manages
// the JS modules we expose to the JavaScript environment.
//
// We treat the modules that represent the JS API code of XWalkExtensions
// specially, since to enable messaging we want to provide a communication
// gateway between RenderViewHandler and these modules. See XWalkExtensionModule
// for details.
class XWalkModuleSystem {
 public:
  XWalkModuleSystem();
  ~XWalkModuleSystem();

  // Functions that manage the relationship between v8::Context and
  // XWalkModuleSystem.
  static XWalkModuleSystem* GetModuleSystemFromContext(
      v8::Handle<v8::Context> context);
  static void SetModuleSystemInContext(
      scoped_ptr<XWalkModuleSystem> module_system,
      v8::Handle<v8::Context> context);
  static void ResetModuleSystemFromContext(v8::Handle<v8::Context> context);

  void RegisterExtensionModule(scoped_ptr<XWalkExtensionModule> module);
  XWalkExtensionModule* GetExtensionModule(const std::string& extension_name);

 private:
  typedef std::map<std::string, XWalkExtensionModule*> ExtensionModuleMap;
  ExtensionModuleMap extension_modules_;

  DISALLOW_COPY_AND_ASSIGN(XWalkModuleSystem);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_MODULE_SYSTEM_H_
