// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace xwalk {
namespace extensions {

namespace {

// Index used to set embedder data into v8::Context, so we can get from a
// context to its corresponding module. Index chosen to not conflict with
// WebCore::V8ContextEmbedderDataField in V8PerContextData.h.
const int kModuleSystemEmbedderDataIndex = 8;

}  // namespace


XWalkModuleSystem::XWalkModuleSystem() {}

XWalkModuleSystem::~XWalkModuleSystem() {}

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

}  // namespace extensions
}  // namespace xwalk
