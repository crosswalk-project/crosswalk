// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_TIZEN_XWALK_WEB_CONTENTS_VIEW_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_TIZEN_XWALK_WEB_CONTENTS_VIEW_DELEGATE_H_

#include "content/public/browser/web_drag_dest_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/context_menu_params.h"
#include "ui/views/widget/widget.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/runtime/browser/tizen/render_view_context_menu_impl.h"

namespace xwalk {

class XWalkWebContentsViewDelegate : public content::WebContentsViewDelegate {
 public:
  XWalkWebContentsViewDelegate(
      content::WebContents* web_contents,
      xwalk::application::ApplicationService* app_service);
  virtual ~XWalkWebContentsViewDelegate();

  // Overridden from WebContentsViewDelegate:
  void ShowContextMenu(content::RenderFrameHost* render_frame_host,
                       const content::ContextMenuParams& params) override;
  content::WebDragDestDelegate* GetDragDestDelegate() override;
  void StoreFocus() override;
  void RestoreFocus() override;
  bool Focus() override;
  void TakeFocus(bool reverse) override;
  void SizeChanged(const gfx::Size& size) override;
  virtual void* CreateRenderWidgetHostViewDelegate(
      content::RenderWidgetHost* render_widget_host);

#if defined(TOOLKIT_VIEWS) || defined(USE_AURA)
  void ShowDisambiguationPopup(
      const gfx::Rect& target_rect,
      const SkBitmap& zoomed_bitmap,
      const gfx::NativeView content,
      const base::Callback<void(ui::GestureEvent*)>& gesture_cb,
      const base::Callback<void(ui::MouseEvent*)>& mouse_cb) override {
    NOTIMPLEMENTED();
  }

  void HideDisambiguationPopup() override { NOTIMPLEMENTED(); }
#endif

 private:
  aura::Window* GetActiveNativeView();
  views::Widget* GetTopLevelWidget();
  views::FocusManager* GetFocusManager();
  void SetInitialFocus();
  void ShowMenu(scoped_ptr<RenderViewContextMenuImpl> menu);

  content::WebContents* web_contents_;
  xwalk::application::ApplicationService* app_service_;
  scoped_ptr<RenderViewContextMenuImpl> context_menu_;

  DISALLOW_COPY_AND_ASSIGN(XWalkWebContentsViewDelegate);
};
}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_TIZEN_XWALK_WEB_CONTENTS_VIEW_DELEGATE_H_
