// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_TOP_VIEW_LAYOUT_VIEWS_H_
#define XWALK_RUNTIME_BROWSER_UI_TOP_VIEW_LAYOUT_VIEWS_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/views/layout/layout_manager.h"

namespace xwalk {

// Layout manager that handle a main content view taking all the space and
// optionally an top view. The top view will always get its preferred
// height. Used for implementing Tizen Indicator when running fullscreen.
//
// This layout expects that the view it is managing have either one or two
// children, and that the setter methods are called accordingly.
class TopViewLayout : public views::LayoutManager {
 public:
  TopViewLayout();
  virtual ~TopViewLayout();

  // Must be set to the content view, that will fill all the remaining available
  // space in the layout. The |content_view| must be child of the view that this
  // layout manages.
  void set_content_view(views::View* content_view) {
    content_view_ = content_view;
  }

  // Optionally sets a top view, that will be positioned on the top and with its
  // preferred height. The |top_view| must be children from the view that this
  // layout manages.
  void set_top_view(views::View* top_view) { top_view_ = top_view; }

  views::View* top_view() const { return top_view_; }

  // Set or check if the top_view_ is in overlay mode. It means that in overlay
  // mode the top_view_ will be over the content_view_.
  void SetUseOverlay(bool enable);
  bool IsUsingOverlay() const;

  // views::LayoutManager implementation.
  virtual void Layout(views::View* host) OVERRIDE;
  virtual gfx::Size GetPreferredSize(views::View* host) OVERRIDE;

 private:
  views::View* top_view_;
  views::View* content_view_;
  bool overlay_;

  DISALLOW_COPY_AND_ASSIGN(TopViewLayout);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_TOP_VIEW_LAYOUT_VIEWS_H_
