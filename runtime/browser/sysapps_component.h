// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_SYSAPPS_COMPONENT_H_
#define XWALK_RUNTIME_BROWSER_SYSAPPS_COMPONENT_H_

#include "base/compiler_specific.h"
#include "xwalk/runtime/browser/xwalk_component.h"
#include "xwalk/sysapps/common/sysapps_manager.h"

namespace xwalk {

class SysAppsComponent : public XWalkComponent {
 public:
  SysAppsComponent();
  virtual ~SysAppsComponent();

  void DisableDeviceCapabilities() { manager_.DisableDeviceCapabilities(); }

 private:
  // XWalkComponent implementation.
  virtual void CreateUIThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) OVERRIDE;
  virtual void CreateExtensionThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) OVERRIDE;

  sysapps::SysAppsManager manager_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_SYSAPPS_COMPONENT_H_
