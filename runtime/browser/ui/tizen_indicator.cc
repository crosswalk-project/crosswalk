// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/tizen_indicator.h"

#include "ui/gfx/canvas.h"
#include "content/public/browser/browser_thread.h"
#include "xwalk/runtime/browser/ui/tizen_plug.h"

namespace {

SkColor kTizenIndicatorColor = SkColorSetARGB(255, 52, 52, 50);

}  // namespace

namespace xwalk {

TizenIndicator::TizenIndicator()
    : plug_(new TizenPlug(this)) {
  if (!plug_->Connect()) {
    plug_.reset();
    return;
  }

  set_background(
      views::Background::CreateSolidBackground(kTizenIndicatorColor));

  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE,
      base::Bind(&TizenPlug::StartWatching, base::Unretained(plug_.get())));
}

TizenIndicator::~TizenIndicator() {
}

bool TizenIndicator::IsConnected() const {
  return plug_;
}

void TizenIndicator::OnPaint(gfx::Canvas* canvas) {
  View::OnPaint(canvas);

  if (image_.isNull())
    return;
  canvas->DrawImageInt(image_, 0, 0);
}

gfx::Size TizenIndicator::GetPreferredSize() {
  return plug_->GetSize();
}

void TizenIndicator::SetImage(const gfx::ImageSkia& img) {
  image_ = img;
  SchedulePaint();
}

}  // namespace xwalk
