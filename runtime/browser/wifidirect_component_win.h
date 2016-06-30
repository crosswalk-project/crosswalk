// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_WIFIDIRECT_COMPONENT_WIN_H_
#define XWALK_RUNTIME_BROWSER_WIFIDIRECT_COMPONENT_WIN_H_

#include "xwalk/runtime/browser/xwalk_component.h"

namespace xwalk {

class WiFiDirectComponent : public XWalkComponent {
 private:
  // XWalkComponent implementation.
  void CreateExtensionThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) override;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_WIFIDIRECT_COMPONENT_WIN_H_
