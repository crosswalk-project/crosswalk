// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/mobile/ui/widget_container_view.h"

namespace xwalk {

WidgetContainerView::WidgetContainerView(views::Widget* widget)
  : widget_(widget) {
  if (widget_)
    widget_->AddObserver(this);
}

WidgetContainerView::~WidgetContainerView() {}

void WidgetContainerView::OnWidgetBoundsChanged(views::Widget* widget,
                                                const gfx::Rect& new_bounds) {
  preferred_size_ = gfx::Size(new_bounds.width(), new_bounds.height());
  PreferredSizeChanged();
}

}  // namespace xwalk
