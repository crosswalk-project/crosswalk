// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_VIEW_EXT_H_
#define XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_VIEW_EXT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/renderer/render_view_observer.h"
#include "third_party/WebKit/public/web/WebPermissionClient.h"

namespace WebKit {

class WebNode;
class WebURL;

}  // namespace WebKit

namespace xwalk {

// Render process side of XWalkRenderViewHostExt, this provides cross-process
// implementation of miscellaneous WebView functions that we need to poke
// WebKit directly to implement (and that aren't needed in the chrome app).
class XWalkRenderViewExt : public content::RenderViewObserver,
                        public WebKit::WebPermissionClient {
 public:
  static void RenderViewCreated(content::RenderView* render_view);

 private:
  explicit XWalkRenderViewExt(content::RenderView* render_view);
  virtual ~XWalkRenderViewExt();

  // RenderView::Observer:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void DidCommitProvisionalLoad(WebKit::WebFrame* frame,
                                        bool is_new_navigation) OVERRIDE;
  virtual void FocusedNodeChanged(const WebKit::WebNode& node) OVERRIDE;
  virtual void DidCommitCompositorFrame() OVERRIDE;

  void OnDocumentHasImagesRequest(int id);

  void OnDoHitTest(int view_x, int view_y);

  void OnSetTextZoomLevel(double zoom_level);

  void OnResetScrollAndScaleState();

  void OnSetInitialPageScale(double page_scale_factor);

  void UpdatePageScaleFactor();

  // WebKit::WebPermissionClient implementation.
  virtual bool allowImage(WebKit::WebFrame* frame,
                          bool enabledPerSettings,
                          const WebKit::WebURL& imageURL) OVERRIDE;

  bool capture_picture_enabled_;

  float page_scale_factor_;

  DISALLOW_COPY_AND_ASSIGN(XWalkRenderViewExt);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_VIEW_EXT_H_
