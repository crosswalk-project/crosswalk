// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/application_component.h"

#include "xwalk/application/browser/application_system.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"

namespace xwalk {

ApplicationComponent::ApplicationComponent(
    XWalkBrowserContext* browser_context)
    : app_system_(application::ApplicationSystem::Create(browser_context)) {
}

ApplicationComponent::~ApplicationComponent() {}

void ApplicationComponent::CreateUIThreadExtensions(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
  if (!XWalkRuntimeFeatures::isApplicationAPIEnabled())
    return;
  app_system_->CreateExtensions(host, extensions);
}

void ApplicationComponent::CreateExtensionThreadExtensions(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
}

}  // namespace xwalk
