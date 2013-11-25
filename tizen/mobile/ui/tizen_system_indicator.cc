// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/mobile/ui/tizen_system_indicator.h"

#include "ui/gfx/canvas.h"
#include "content/public/browser/browser_thread.h"
#include "xwalk/tizen/mobile/ui/tizen_system_indicator_watcher.h"

#include "ui/views/widget/native_widget.h"
#include "ui/views/widget/root_view.h"
#include "ui/aura/root_window.h"

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

bool TizenSystemIndicator::OnMousePressed(const ui::MouseEvent& event) {
  watcher_->OnMouseDown();
  return true;
}

void TizenSystemIndicator::OnMouseReleased(const ui::MouseEvent& event) {
  watcher_->OnMouseUp();
}

void TizenSystemIndicator::OnTouchEvent(ui::TouchEvent* event) {
  const gfx::Point position = event->location();

  switch (event->type()) {
    case ui::ET_UNKNOWN:
    case ui::ET_MOUSE_PRESSED:
    case ui::ET_MOUSE_DRAGGED:
    case ui::ET_MOUSE_RELEASED:
    case ui::ET_MOUSE_MOVED:
    case ui::ET_MOUSE_ENTERED:
    case ui::ET_MOUSE_EXITED:
    case ui::ET_KEY_PRESSED:
    case ui::ET_KEY_RELEASED:
    case ui::ET_MOUSEWHEEL:
    case ui::ET_MOUSE_CAPTURE_CHANGED:
    case ui::ET_TOUCH_RELEASED:
      watcher_->OnMouseUp();
      break;
    case ui::ET_TOUCH_PRESSED:
      watcher_->OnMouseDown();
      break;
    case ui::ET_TOUCH_MOVED:
      watcher_->OnMouseMove(position.x(), position.y());
      break;
    case ui::ET_TOUCH_STATIONARY:
    case ui::ET_TOUCH_CANCELLED:
    case ui::ET_DROP_TARGET_EVENT:
    case ui::ET_TRANSLATED_KEY_PRESS:
    case ui::ET_TRANSLATED_KEY_RELEASE:
    case ui::ET_GESTURE_SCROLL_BEGIN:
    case ui::ET_GESTURE_SCROLL_END:
    case ui::ET_GESTURE_SCROLL_UPDATE:
    case ui::ET_GESTURE_TAP:
    case ui::ET_GESTURE_TAP_DOWN:
    case ui::ET_GESTURE_TAP_CANCEL:
    case ui::ET_GESTURE_BEGIN:
    case ui::ET_GESTURE_END:
    case ui::ET_GESTURE_TWO_FINGER_TAP:
    case ui::ET_GESTURE_PINCH_BEGIN:
    case ui::ET_GESTURE_PINCH_END:
    case ui::ET_GESTURE_PINCH_UPDATE:
    case ui::ET_GESTURE_LONG_PRESS:
    case ui::ET_GESTURE_LONG_TAP:
    case ui::ET_GESTURE_MULTIFINGER_SWIPE:
    case ui::ET_GESTURE_SHOW_PRESS:
    case ui::ET_SCROLL:
    case ui::ET_SCROLL_FLING_START:
    case ui::ET_SCROLL_FLING_CANCEL:
    case ui::ET_CANCEL_MODE:
    case ui::ET_UMA_DATA:
    case ui::ET_LAST:
      break;
  }
  event->SetHandled();
}

void TizenSystemIndicator::OnMouseMoved(const ui::MouseEvent& event) {
  const gfx::Point position = event.location();
  watcher_->OnMouseMove(position.x(), position.y());
}

}  // namespace xwalk
