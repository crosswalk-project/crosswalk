// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_EXTENSIONS_RENDERER_CAMEO_EXTENSION_RENDER_VIEW_HANDLER_H_
#define CAMEO_EXTENSIONS_RENDERER_CAMEO_EXTENSION_RENDER_VIEW_HANDLER_H_

#include <string>
#include "content/public/renderer/render_view_observer.h"
#include "content/public/renderer/render_view_observer_tracker.h"

namespace cameo {
namespace extensions {

class CameoExtensionRendererController;

// This helper object is associated with each RenderView and handles the message
// exchange between browser process and the JavaScript code.
class CameoExtensionRenderViewHandler
    : public content::RenderViewObserver,
      public
      content::RenderViewObserverTracker<CameoExtensionRenderViewHandler> {
 public:
  CameoExtensionRenderViewHandler(content::RenderView* render_view,
                          CameoExtensionRendererController* controller);

  // Get the handler for the current context. Used in v8::Extension.
  // This convenience is one of the reasons to have this helper class.
  static CameoExtensionRenderViewHandler* GetForCurrentContext();

  bool PostMessageToExtension(const std::string& extension,
                              const std::string& msg);

  // RenderViewObserver implementation.
  virtual void DidClearWindowObject(WebKit::WebFrame* frame) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  // Called when we receive a message from the browser process, dispatches it to
  // JavaScript environment.
  void OnPostMessage(const std::string& extension, const std::string& msg);

  CameoExtensionRendererController* controller_;

  DISALLOW_COPY_AND_ASSIGN(CameoExtensionRenderViewHandler);
};

}  // namespace extensions
}  // namespace cameo

#endif  // CAMEO_EXTENSIONS_RENDERER_CAMEO_EXTENSION_RENDER_VIEW_HANDLER_H_
