// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/tizen_system_indicator.h"

#include "ui/gfx/canvas.h"
#include "content/public/browser/browser_thread.h"
#include "xwalk/runtime/browser/ui/tizen_system_indicator_watcher.h"

namespace {

SkColor kBGColor = SkColorSetARGB(255, 52, 52, 50);

}  // namespace

namespace xwalk {

TizenSystemIndicator::TizenSystemIndicator()
    : watcher_(new TizenSystemIndicatorWatcher(this)) {
  if (!watcher_->Connect()) {
    watcher_.reset();
    return;
  }

  set_background(views::Background::CreateSolidBackground(kBGColor));

  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE,
      base::Bind(&TizenSystemIndicatorWatcher::StartWatching,
                 base::Unretained(watcher_.get())));
}

TizenSystemIndicator::~TizenSystemIndicator() {
}

bool TizenSystemIndicator::IsConnected() const {
  return watcher_;
}

void TizenSystemIndicator::OnPaint(gfx::Canvas* canvas) {
  View::OnPaint(canvas);

  if (image_.isNull())
    return;
  canvas->DrawImageInt(image_, 0, 0);
}

gfx::Size TizenSystemIndicator::GetPreferredSize() {
  return watcher_->GetSize();
}

void TizenSystemIndicator::SetImage(const gfx::ImageSkia& img) {
  image_ = img;
  SchedulePaint();
}

}  // namespace xwalk
