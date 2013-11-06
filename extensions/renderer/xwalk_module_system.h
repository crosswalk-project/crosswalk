// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_MODULE_SYSTEM_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_MODULE_SYSTEM_H_

#include <map>
#include <vector>
#include <string>
#include "base/values.h"
#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionModule;

// Interface used to expose objects via the requireNative() function in JS API
// code. Native modules should be registered with the module system.
class XWalkNativeModule {
 public:
  virtual v8::Handle<v8::Object> NewInstance() = 0;
  virtual ~XWalkNativeModule() {}
};

// This object is associated with the v8::Context of each WebFrame. It manages
// the JS modules we expose to the JavaScript environment.
//
// We treat the modules that represent the JS API code of XWalkExtensions
// specially, since to enable messaging we want to provide a communication
// gateway between RenderViewHandler and these modules. See XWalkExtensionModule
// for details.
class XWalkModuleSystem {
 public:
  explicit XWalkModuleSystem(v8::Handle<v8::Context> context);
  ~XWalkModuleSystem();

  // Functions that manage the relationship between v8::Context and
  // XWalkModuleSystem.
  static XWalkModuleSystem* GetModuleSystemFromContext(
      v8::Handle<v8::Context> context);
  static void SetModuleSystemInContext(
      scoped_ptr<XWalkModuleSystem> module_system,
      v8::Handle<v8::Context> context);
  static void ResetModuleSystemFromContext(v8::Handle<v8::Context> context);

  void RegisterExtensionModule(scoped_ptr<XWalkExtensionModule> module,
                               const std::vector<std::string>& entry_points);

  void RegisterNativeModule(const std::string& name,
                            scoped_ptr<XWalkNativeModule> module);
  v8::Handle<v8::Object> RequireNative(const std::string& name);

  void Initialize();

  v8::Handle<v8::Context> GetV8Context();

 private:
  struct ExtensionModuleEntry {
    ExtensionModuleEntry(const std::string& name, XWalkExtensionModule* module,
                         const std::vector<std::string>& entry_points);
    ~ExtensionModuleEntry();
    std::string name;
    XWalkExtensionModule* module;
    bool use_trampoline;
    std::vector<std::string> entry_points;
    bool operator<(const ExtensionModuleEntry& other) const {
      return name < other.name;
    }

    static bool IsPrefix(const ExtensionModuleEntry& first,
                         const ExtensionModuleEntry& second);
  };

  bool SetTrampolineAccessorForEntryPoint(
      v8::Handle<v8::Context> context,
      const std::string& entry_point,
      v8::Local<v8::External> user_data);

  static bool DeleteAccessorForEntryPoint(v8::Handle<v8::Context> context,
                                          const std::string& entry_point);

  bool InstallTrampoline(v8::Handle<v8::Context> context,
                         ExtensionModuleEntry* entry);

  static void TrampolineCallback(
      v8::Local<v8::String> property,
      const v8::PropertyCallbackInfo<v8::Value>& info);

  bool ContainsEntryPoint(const std::string& entry_point);
  void MarkModulesWithTrampoline();
  void DeleteExtensionModules();

  typedef std::vector<ExtensionModuleEntry> ExtensionModules;
  ExtensionModules extension_modules_;

  typedef std::map<std::string, XWalkNativeModule*> NativeModuleMap;
  NativeModuleMap native_modules_;

  v8::Persistent<v8::FunctionTemplate> require_native_template_;

  v8::Persistent<v8::Object> function_data_;

  // Points back to the current context, used when native wants to callback
  // JavaScript. When WillReleaseScriptContext() is called, we dispose this
  // persistent.
  v8::Persistent<v8::Context> v8_context_;

  DISALLOW_COPY_AND_ASSIGN(XWalkModuleSystem);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_MODULE_SYSTEM_H_
