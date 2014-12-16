// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_TIZEN_RENDER_VIEW_CONTEXT_MENU_IMPL_H_
#define XWALK_RUNTIME_BROWSER_TIZEN_RENDER_VIEW_CONTEXT_MENU_IMPL_H_

#include "components/renderer_context_menu/render_view_context_menu_base.h"

#include "content/public/browser/render_widget_host_view.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/window.h"
#include "ui/views/widget/widget.h"

namespace aura {
class Window;
}

namespace views {
class Widget;
}

namespace gfx {
class Point;
}

namespace xwalk {

class RenderViewContextMenuImpl : public RenderViewContextMenuBase {
 public:
  RenderViewContextMenuImpl(content::RenderFrameHost* render_frame_host,
                            const content::ContextMenuParams& params);
  virtual ~RenderViewContextMenuImpl();

  void RunMenuAt(views::Widget* parent,
                 const gfx::Point& point,
                 ui::MenuSourceType type);

  void Show() override;

 private:
  // RenderViewContextMenuBase:
  void InitMenu() override;
  void RecordShownItem(int id) override;
  void RecordUsedItem(int id) override;
  void NotifyMenuShown() override;
  void NotifyURLOpened(const GURL& url,
                               content::WebContents* new_contents) override;
  void HandleAuthorizeAllPlugins() override;

  // ui::SimpleMenuModel:
  bool GetAcceleratorForCommandId(
      int command_id,
      ui::Accelerator* accelerator) override;
  bool IsCommandIdChecked(int command_id) const override;
  bool IsCommandIdEnabled(int command_id) const override;
  void ExecuteCommand(int command_id, int event_flags) override;

  aura::Window* GetActiveNativeView();
  views::Widget* GetTopLevelWidget();

  DISALLOW_COPY_AND_ASSIGN(RenderViewContextMenuImpl);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_TIZEN_RENDER_VIEW_CONTEXT_MENU_IMPL_H_
