// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_

#include <map>
#include <string>
#include <vector>
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/render_process_observer.h"
#include "v8/include/v8.h"

namespace content {
class RenderView;
}

namespace IPC {
class ChannelHandle;
}

namespace WebKit {
class WebFrame;
}

namespace xwalk {
namespace extensions {

class XWalkExtensionClient;

// Renderer controller for XWalk extensions keeps track of the extensions
// registered into the system. It also watches for new render views to attach
// the extensions handlers to them.
class XWalkExtensionRendererController : public content::RenderProcessObserver {
 public:
  XWalkExtensionRendererController();
  virtual ~XWalkExtensionRendererController();

  // To be called in XWalkContentRendererClient so we can create and
  // destroy extensions contexts appropriatedly.
  void DidCreateScriptContext(WebKit::WebFrame* frame,
                              v8::Handle<v8::Context> context);
  void WillReleaseScriptContext(WebKit::WebFrame* frame,
                                v8::Handle<v8::Context> context);

  // RenderProcessObserver implementation.
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  // Message Handlers.
  void OnExtensionProcessChannelCreated(const IPC::ChannelHandle& handle);

  scoped_ptr<XWalkExtensionClient> in_browser_process_extensions_client_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionRendererController);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_
