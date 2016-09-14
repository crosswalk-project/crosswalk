// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_VIEW_EXT_H_
#define XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_VIEW_EXT_H_

#include "base/compiler_specific.h"
#include "content/public/renderer/render_view_observer.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/size_f.h"


namespace blink {

class WebNode;
class WebURL;

}  // namespace blink

namespace xwalk {

// Render process side of XWalkRenderViewHostExt, this provides cross-process
// implementation of miscellaneous WebView functions that we need to poke
// WebKit directly to implement (and that aren't needed in the chrome app).
class XWalkRenderViewExt : public content::RenderViewObserver {
 public:
  static void RenderViewCreated(content::RenderView* render_view);

 private:
  explicit XWalkRenderViewExt(content::RenderView* render_view);
  ~XWalkRenderViewExt() override;

  // RenderView::Observer:
  bool OnMessageReceived(const IPC::Message& message) override;
  void DidCommitProvisionalLoad(blink::WebLocalFrame* frame,
                                bool is_new_navigation) override;
  void FocusedNodeChanged(const blink::WebNode& node) override;
  void OnDestruct() override;

  void OnDocumentHasImagesRequest(int id);

  void OnDoHitTest(const gfx::PointF& touch_center,
                   const gfx::SizeF& touch_area);

  void OnSetTextZoomLevel(double zoom_level);

  void OnResetScrollAndScaleState();

  void OnSetInitialPageScale(double page_scale_factor);

  void OnSetBackgroundColor(SkColor c);

  void OnSetTextZoomFactor(float zoom_factor);

  DISALLOW_COPY_AND_ASSIGN(XWalkRenderViewExt);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_VIEW_EXT_H_
