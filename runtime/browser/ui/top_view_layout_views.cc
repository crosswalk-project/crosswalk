// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/top_view_layout_views.h"

#include "ui/views/view.h"

namespace xwalk {

TopViewLayout::TopViewLayout()
    : top_view_(NULL),
      content_view_(NULL),
      overlay_(false) {}

TopViewLayout::~TopViewLayout() {}

void TopViewLayout::SetUseOverlay(bool enable) {
  if (overlay_ == enable)
    return;
  overlay_ = enable;
}

bool TopViewLayout::IsUsingOverlay() const {
  return overlay_;
}

void TopViewLayout::Layout(views::View* host) {
  if (!host->has_children())
    return;

  if (!top_view_) {
    content_view_->SetBoundsRect(host->GetLocalBounds());
    return;
  }

  CHECK_EQ(2, host->child_count());
  CHECK(content_view_);
  CHECK_EQ(host, top_view_->parent());
  CHECK_EQ(host, content_view_->parent());

  gfx::Rect top_view_bounds = host->GetLocalBounds();
  top_view_bounds.set_width(top_view_->GetPreferredSize().width());
  top_view_bounds.set_height(top_view_->GetPreferredSize().height());
  top_view_->SetBoundsRect(top_view_bounds);

  gfx::Rect content_view_bounds = host->GetLocalBounds();
  if (overlay_)
    content_view_bounds.Inset(0, 0, 0, 0);
  else
    content_view_bounds.Inset(0, top_view_bounds.height(), 0, 0);
  content_view_->SetBoundsRect(content_view_bounds);
}

gfx::Size TopViewLayout::GetPreferredSize(views::View* host) {
  CHECK(content_view_);
  gfx::Rect rect(content_view_->GetPreferredSize());
  if (top_view_)
    rect.Inset(0, -top_view_->GetPreferredSize().height(), 0, 0);
  return rect.size();
}

}  // namespace xwalk
