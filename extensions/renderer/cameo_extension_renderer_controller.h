// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_EXTENSIONS_RENDERER_CAMEO_EXTENSION_RENDERER_CONTROLLER_H_
#define CAMEO_EXTENSIONS_RENDERER_CAMEO_EXTENSION_RENDERER_CONTROLLER_H_

#include <map>
#include <string>
#include "base/compiler_specific.h"
#include "content/public/renderer/render_process_observer.h"

namespace content {
class RenderView;
}

namespace WebKit {
class WebFrame;
}

namespace cameo {
namespace extensions {

// Renderer controller for Cameo extensions keeps track of the extensions
// registered into the system. It also watches for new render views to attach
// the extensions handlers to them.
class CameoExtensionRendererController : public content::RenderProcessObserver {
 public:
  CameoExtensionRendererController();
  virtual ~CameoExtensionRendererController();

  // To be called by client code when a render view is created. Will attach
  // extension handlers to them.
  void RenderViewCreated(content::RenderView* render_view);

  // RenderProcessObserver implementation.
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  friend class CameoExtensionRenderViewHandler;

  // Called when browser process send a message with a new extension to be
  // registered, and its corresponding JavaScript API.
  void OnRegisterExtension(const std::string& extension,
                           const std::string& api);

  // Returns whether the extension was already registered in the controller.
  bool ContainsExtension(const std::string& extension) const;

  // Installs the extensions' JavaScript API code into the given frame.
  void InstallJavaScriptAPIs(WebKit::WebFrame* frame);

  typedef std::map<std::string, std::string> ExtensionAPIMap;
  ExtensionAPIMap extension_apis_;

  DISALLOW_COPY_AND_ASSIGN(CameoExtensionRendererController);
};

}  // namespace extensions
}  // namespace cameo

#endif  // CAMEO_EXTENSIONS_RENDERER_CAMEO_EXTENSION_RENDERER_CONTROLLER_H_
