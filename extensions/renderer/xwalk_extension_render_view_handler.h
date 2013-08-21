// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDER_VIEW_HANDLER_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDER_VIEW_HANDLER_H_

#include <stdint.h>
#include <map>
#include <string>
#include "base/values.h"
#include "content/public/renderer/render_view_observer.h"
#include "content/public/renderer/render_view_observer_tracker.h"
#include "v8/include/v8.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionRendererController;

// This helper object is associated with each RenderView and handles the message
// exchange between browser process and the JavaScript code. One handler takes
// care of all the WebFrames of the render view.
class XWalkExtensionRenderViewHandler
    : public content::RenderViewObserver,
      public
      content::RenderViewObserverTracker<XWalkExtensionRenderViewHandler> {
 public:
  XWalkExtensionRenderViewHandler(content::RenderView* render_view,
                          XWalkExtensionRendererController* controller);
  virtual ~XWalkExtensionRenderViewHandler();

  // Get the handler for the current context. Used in v8::Extension.
  // This convenience is one of the reasons to have this helper class.
  static XWalkExtensionRenderViewHandler* GetForCurrentContext();

  static XWalkExtensionRenderViewHandler* GetForFrame(
      WebKit::WebFrame* webframe);

  bool PostMessageToExtension(
      int64_t frame_id, const std::string& extension,
      scoped_ptr<base::Value> msg);
  scoped_ptr<base::Value> SendSyncMessageToExtension(
      int64_t frame_id, const std::string& extension,
      scoped_ptr<base::Value> msg);

  void DidCreateScriptContext(WebKit::WebFrame* frame);
  void WillReleaseScriptContext(WebKit::WebFrame* frame);

  // RenderViewObserver implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  // Called when we receive a message from the browser process, dispatches it to
  // JavaScript environment.
  void OnPostMessage(int64_t frame_id, const std::string& extension_name,
                     const base::ListValue& msg);

  v8::Handle<v8::Context> GetV8ContextForFrame(int64_t frame_id);

  XWalkExtensionRendererController* controller_;

  typedef std::map<int64_t, WebKit::WebFrame*> IdToFrameMap;
  IdToFrameMap id_to_frame_map_;


  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionRenderViewHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDER_VIEW_HANDLER_H_
