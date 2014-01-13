// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_COMPONENT_H_
#define XWALK_RUNTIME_BROWSER_XWALK_COMPONENT_H_

#include "xwalk/extensions/common/xwalk_extension_vector.h"

namespace content {
class RenderProcessHost;
}

namespace xwalk {

// Base class for subsystems of Crosswalk to hook into important
// events of "Browser Process" execution. The concrete implementations
// are instantiated by XWalkRunner before the message loop starts and
// destroyed right after the message loop ends.
class XWalkComponent {
 public:
  virtual ~XWalkComponent() {}

  // TODO(cmarcelo): Replace RPH with some reference to Application.
  virtual void CreateUIThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) {}

  virtual void CreateExtensionThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) {}

 protected:
  XWalkComponent() {}
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_COMPONENT_H_
