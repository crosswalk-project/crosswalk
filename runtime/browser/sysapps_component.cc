// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/sysapps_component.h"

#include "xwalk/runtime/common/xwalk_runtime_features.h"

namespace xwalk {

SysAppsComponent::SysAppsComponent() {
  if (!XWalkRuntimeFeatures::isDeviceCapabilitiesAPIEnabled())
    manager_.DisableDeviceCapabilities();
  if (!XWalkRuntimeFeatures::isRawSocketsAPIEnabled())
    manager_.DisableRawSockets();
}

SysAppsComponent::~SysAppsComponent() {}

void SysAppsComponent::CreateUIThreadExtensions(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
  manager_.CreateExtensionsForUIThread(extensions);
}

void SysAppsComponent::CreateExtensionThreadExtensions(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
  manager_.CreateExtensionsForExtensionThread(extensions);
}

}  // namespace xwalk
