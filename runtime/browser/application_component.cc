// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/application_component.h"

#include "xwalk/application/browser/application_system.h"

namespace xwalk {

ApplicationComponent::ApplicationComponent(
    RuntimeContext* runtime_context)
    : app_system_(application::ApplicationSystem::Create(runtime_context)),
      extensions_enabled_(true) {}

ApplicationComponent::~ApplicationComponent() {}

void ApplicationComponent::CreateUIThreadExtensions(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
  if (!extensions_enabled_)
    return;
  app_system_->CreateExtensions(host, extensions);
}

void ApplicationComponent::CreateExtensionThreadExtensions(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {}

}  // namespace xwalk
