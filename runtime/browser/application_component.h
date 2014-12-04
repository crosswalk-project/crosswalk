// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_APPLICATION_COMPONENT_H_
#define XWALK_RUNTIME_BROWSER_APPLICATION_COMPONENT_H_

#include "base/memory/scoped_ptr.h"
#include "xwalk/runtime/browser/xwalk_component.h"

namespace xwalk {

class XWalkBrowserContext;

namespace application {
class ApplicationSystem;
}

// Sets up the ApplicationSystem features of Crosswalk, and implements
// the necessary interface to register the extensions for new
// Applications being launched.
class ApplicationComponent : public XWalkComponent {
 public:
  explicit ApplicationComponent(XWalkBrowserContext* browser_context);
  virtual ~ApplicationComponent();

  // Used by Android since extensions for Application are not supported there.
  void DisableExtensions() { extensions_enabled_ = false; }

  application::ApplicationSystem* app_system() { return app_system_.get(); }

 private:
  // XWalkComponent implementation.
  void CreateUIThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) override;
  void CreateExtensionThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) override;

  scoped_ptr<application::ApplicationSystem> app_system_;
  bool extensions_enabled_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_APPLICATION_COMPONENT_H_
