// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_TIZEN_XWALK_RENDER_VIEW_EXT_TIZEN_H_
#define XWALK_RUNTIME_RENDERER_TIZEN_XWALK_RENDER_VIEW_EXT_TIZEN_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/renderer/render_view_observer.h"

namespace xwalk {

class XWalkRenderViewExtTizen : public content::RenderViewObserver {
 public:
  static void RenderViewCreated(content::RenderView* render_view);

 private:
  explicit XWalkRenderViewExtTizen(content::RenderView* render_view);
  virtual ~XWalkRenderViewExtTizen();

  // RenderView::Observer:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void OnHWKeyPressed(int keycode);

  content::RenderView* render_view_;
  DISALLOW_COPY_AND_ASSIGN(XWalkRenderViewExtTizen);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_TIZEN_XWALK_RENDER_VIEW_EXT_TIZEN_H_
