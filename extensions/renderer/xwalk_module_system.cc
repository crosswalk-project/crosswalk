// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_module_system.h"

#include "base/logging.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"

namespace xwalk {
namespace extensions {

namespace {

// Index used to set embedder data into v8::Context, so we can get from a
// context to its corresponding module. Index chosen to not conflict with
// WebCore::V8ContextEmbedderDataField in V8PerContextData.h.
const int kModuleSystemEmbedderDataIndex = 8;

}  // namespace


XWalkModuleSystem::XWalkModuleSystem() {}

XWalkModuleSystem::~XWalkModuleSystem() {
  ExtensionModuleMap::iterator it = extension_modules_.begin();
  for (; it != extension_modules_.end(); ++it)
    delete it->second;
  extension_modules_.clear();
}

// static
XWalkModuleSystem* XWalkModuleSystem::GetModuleSystemFromContext(
    v8::Handle<v8::Context> context) {
  return reinterpret_cast<XWalkModuleSystem*>(
      context->GetAlignedPointerFromEmbedderData(
          kModuleSystemEmbedderDataIndex));
}

// static
void XWalkModuleSystem::SetModuleSystemInContext(
    scoped_ptr<XWalkModuleSystem> module_system,
    v8::Handle<v8::Context> context) {
  context->SetAlignedPointerInEmbedderData(kModuleSystemEmbedderDataIndex,
                                           module_system.release());
}

// static
void XWalkModuleSystem::ResetModuleSystemFromContext(
    v8::Handle<v8::Context> context) {
  delete GetModuleSystemFromContext(context);
  SetModuleSystemInContext(scoped_ptr<XWalkModuleSystem>(), context);
}

void XWalkModuleSystem::RegisterExtensionModule(
    scoped_ptr<XWalkExtensionModule> module) {
  const std::string& extension_name = module->extension_name();
  CHECK(extension_modules_.find(extension_name) == extension_modules_.end());
  extension_modules_[extension_name] = module.release();
}

XWalkExtensionModule* XWalkModuleSystem::GetExtensionModule(
    const std::string& extension_name) {
  ExtensionModuleMap::iterator it = extension_modules_.find(extension_name);
  CHECK(it != extension_modules_.end());
  return it->second;
}

}  // namespace extensions
}  // namespace xwalk
