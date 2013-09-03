// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_

#include <map>
#include <string>
#include <vector>
#include "base/compiler_specific.h"
#include "content/public/renderer/render_process_observer.h"
#include "v8/include/v8.h"

namespace content {
class RenderView;
}

namespace WebKit {
class WebFrame;
}

namespace xwalk {
namespace extensions {

class XWalkExtensionRenderViewHandler;
class XWalkRemoteExtensionRunner;

// Renderer controller for XWalk extensions keeps track of the extensions
// registered into the system. It also watches for new render views to attach
// the extensions handlers to them.
class XWalkExtensionRendererController : public content::RenderProcessObserver {
 public:
  XWalkExtensionRendererController();
  virtual ~XWalkExtensionRendererController();

  // To be called by client code when a render view is created. Will attach
  // extension handlers to them.
  void RenderViewCreated(content::RenderView* render_view);

  // To be called in XWalkContentRendererClient so we can create and
  // destroy extensions contexts appropriatedly.
  void DidCreateScriptContext(WebKit::WebFrame* frame,
                              v8::Handle<v8::Context> context);
  void WillReleaseScriptContext(WebKit::WebFrame* frame,
                                v8::Handle<v8::Context> context);

  // RenderProcessObserver implementation.
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

  // FIXME(cmarcelo): Remove me.
  XWalkRemoteExtensionRunner* GetRunner(
      XWalkExtensionRenderViewHandler* handler, int64_t frame_id,
      const std::string& extension_name);

 private:
  friend class XWalkExtensionRenderViewHandler;

  // Called when browser process send a message with a new extension to be
  // registered, and its corresponding JavaScript API.
  void OnRegisterExtension(const std::string& extension,
                           const std::string& api);

  // Returns whether the extension was already registered in the controller.
  bool ContainsExtension(const std::string& extension) const;

  typedef std::map<std::string, std::string> ExtensionAPIMap;
  ExtensionAPIMap extension_apis_;

  // FIXME(cmarcelo): Modify to be a map. Move inside XWalkExtensionClient.
  typedef std::vector<XWalkRemoteExtensionRunner*> RunnerVector;
  RunnerVector runners_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionRendererController);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_
