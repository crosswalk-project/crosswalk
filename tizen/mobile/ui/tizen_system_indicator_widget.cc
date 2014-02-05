// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/mobile/ui/tizen_system_indicator_widget.h"

#include "content/public/browser/browser_thread.h"
#include "ui/views/background.h"

namespace {

SkColor kBGColor = SkColorSetARGB(255, 52, 52, 50);

}  // namespace

namespace xwalk {

TizenSystemIndicatorWidget::TizenSystemIndicatorWidget()
  : indicator_(new TizenSystemIndicator()) {
}

TizenSystemIndicatorWidget::~TizenSystemIndicatorWidget() {}

void TizenSystemIndicatorWidget::Initialize(gfx::NativeView parent) {
  views::Widget::InitParams params;
  params.parent = parent;
  params.type = views::Widget::InitParams::TYPE_WINDOW_FRAMELESS;
  params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
  params.keep_on_top = true;
  Init(params);
  SetContentsView(indicator_.get());
  Show();
}

void TizenSystemIndicatorWidget::OnImageUpdated(
    const gfx::ImageSkia& img_skia) {
  indicator_->SetImage(img_skia);

  gfx::Rect indicator_bounds;
  indicator_bounds.set_width(indicator_->GetPreferredSize().width());
  indicator_bounds.set_height(indicator_->GetPreferredSize().height());
  indicator_->SetBoundsRect(indicator_bounds);
  SetBounds(indicator_bounds);
}

void TizenSystemIndicatorWidget::SetDisplay(const gfx::Display& display) {
  indicator_->SetImage(0);

  watcher_.reset(new TizenSystemIndicatorWatcher(this, display));
  if (!watcher_->Connect()) {
    watcher_.reset();
    indicator_->set_background(NULL);
    indicator_->SetWatcher(NULL);
    return;
  }

  if (display.rotation() == gfx::Display::ROTATE_0 ||
      display.rotation() == gfx::Display::ROTATE_180) {
    indicator_->set_background(
      views::Background::CreateSolidBackground(kBGColor));
  } else {
    indicator_->set_background(NULL);
  }

  indicator_->SetWatcher(watcher_.get());

  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE,
      base::Bind(&TizenSystemIndicatorWatcher::StartWatching,
                 base::Unretained(watcher_.get())));
}

}  // namespace xwalk
