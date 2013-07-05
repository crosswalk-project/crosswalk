// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDER_VIEW_HANDLER_H_
#define CAMEO_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDER_VIEW_HANDLER_H_

#include <string>
#include "content/public/renderer/render_view_observer.h"
#include "content/public/renderer/render_view_observer_tracker.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionRendererController;

// This helper object is associated with each RenderView and handles the message
// exchange between browser process and the JavaScript code.
class XWalkExtensionRenderViewHandler
    : public content::RenderViewObserver,
      public
      content::RenderViewObserverTracker<XWalkExtensionRenderViewHandler> {
 public:
  XWalkExtensionRenderViewHandler(content::RenderView* render_view,
                          XWalkExtensionRendererController* controller);

  // Get the handler for the current context. Used in v8::Extension.
  // This convenience is one of the reasons to have this helper class.
  static XWalkExtensionRenderViewHandler* GetForCurrentContext();

  bool PostMessageToExtension(const std::string& extension,
                              const std::string& msg);

  // RenderViewObserver implementation.
  virtual void DidClearWindowObject(WebKit::WebFrame* frame) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  // Called when we receive a message from the browser process, dispatches it to
  // JavaScript environment.
  void OnPostMessage(const std::string& extension, const std::string& msg);

  XWalkExtensionRendererController* controller_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionRenderViewHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // CAMEO_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDER_VIEW_HANDLER_H_
