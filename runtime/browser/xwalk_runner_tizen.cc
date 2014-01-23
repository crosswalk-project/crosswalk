// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner_tizen.h"

#include "xwalk/runtime/browser/sysapps_component.h"
#include "xwalk/runtime/browser/xwalk_component.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_extension.h"

namespace xwalk {

XWalkRunnerTizen::XWalkRunnerTizen() {}

XWalkRunnerTizen::~XWalkRunnerTizen() {}

// static
XWalkRunnerTizen* XWalkRunnerTizen::GetInstance() {
  return static_cast<XWalkRunnerTizen*>(XWalkRunner::GetInstance());
}

namespace {

// TODO(cmarcelo): See comment below in CreateSysAppsComponent.
class DeviceCapabilitiesComponent : public XWalkComponent {
 public:
  DeviceCapabilitiesComponent() {}
  virtual ~DeviceCapabilitiesComponent() {}

  // XWalkComponent implementation.
  virtual void CreateExtensionThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) OVERRIDE {
    extensions->push_back(new sysapps::DeviceCapabilitiesExtension());
  }
};

}  // namespace

void XWalkRunnerTizen::CreateComponents() {
  XWalkRunner::CreateComponents();
  if (XWalkRuntimeFeatures::isDeviceCapabilitiesAPIEnabled()) {
    AddComponent(scoped_ptr<XWalkComponent>(new DeviceCapabilitiesComponent));
  }
}

scoped_ptr<SysAppsComponent> XWalkRunnerTizen::CreateSysAppsComponent() {
  scoped_ptr<SysAppsComponent> sysapps_component(
      XWalkRunner::CreateSysAppsComponent());
  // TODO(cmarcelo): In Tizen we still use an old DeviceCapabilities, when the
  // new version achieves feature parity we can remove the old and stop
  // disabling the new.
  sysapps_component->DisableDeviceCapabilities();
  return sysapps_component.Pass();
}

}  // namespace xwalk
