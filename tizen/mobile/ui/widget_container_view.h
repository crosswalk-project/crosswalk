// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_MOBILE_UI_WIDGET_CONTAINER_VIEW_H_
#define XWALK_TIZEN_MOBILE_UI_WIDGET_CONTAINER_VIEW_H_

#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_observer.h"

namespace xwalk {

// This view doesn't draw anything, just contains a widget and trigger the
// parent's Layout() when the widget bounds are changed.
class WidgetContainerView : public views::View, public views::WidgetObserver {
 public:
  explicit WidgetContainerView(views::Widget* widget);
  virtual ~WidgetContainerView();

  // View implementation.
  virtual gfx::Size GetPreferredSize() OVERRIDE { return preferred_size_; }

  // views::WidgetObserver implementation.
  virtual void OnWidgetBoundsChanged(views::Widget* widget,
                                     const gfx::Rect& new_bounds) OVERRIDE;
 private:
  views::Widget* widget_;
  gfx::Size preferred_size_;
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_MOBILE_UI_WIDGET_CONTAINER_VIEW_H_
