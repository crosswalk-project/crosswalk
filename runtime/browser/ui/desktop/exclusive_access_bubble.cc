// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop/exclusive_access_bubble.h"

#include "base/strings/utf_string_conversions.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/strings/grit/ui_strings.h"
#include "xwalk/grit/xwalk_resources.h"

namespace xwalk {

// NOTE(koz): Linux doesn't use the thick shadowed border, so we add padding
// here.
#if defined(OS_LINUX)
const int ExclusiveAccessBubble::kPaddingPx = 8;
#else
const int ExclusiveAccessBubble::kPaddingPx = 15;
#endif
const int ExclusiveAccessBubble::kInitialDelayMs = 3800;
const int ExclusiveAccessBubble::kIdleTimeMs = 2300;
const int ExclusiveAccessBubble::kSnoozeNotificationsTimeMs = 900000;  // 15m.
const int ExclusiveAccessBubble::kPositionCheckHz = 10;
const int ExclusiveAccessBubble::kSlideInRegionHeightPx = 4;
const int ExclusiveAccessBubble::kSlideInDurationMs = 350;
const int ExclusiveAccessBubble::kSlideOutDurationMs = 700;
const int ExclusiveAccessBubble::kPopupTopPx = 45;

ExclusiveAccessBubble::ExclusiveAccessBubble(
    Type bubble_type)
    : bubble_type_(bubble_type) {
  DCHECK_NE(TYPE_NONE, bubble_type_);
}

ExclusiveAccessBubble::~ExclusiveAccessBubble() {
}

void ExclusiveAccessBubble::StartWatchingMouse() {
  // Start the initial delay timer and begin watching the mouse.
  hide_timeout_.Start(FROM_HERE,
                      base::TimeDelta::FromMilliseconds(kInitialDelayMs),
                      this, &ExclusiveAccessBubble::CheckMousePosition);

  gfx::Point cursor_pos = GetCursorScreenPoint();
  last_mouse_pos_ = cursor_pos;
  mouse_position_checker_.Start(
      FROM_HERE, base::TimeDelta::FromMilliseconds(1000 / kPositionCheckHz),
      this, &ExclusiveAccessBubble::CheckMousePosition);
}

void ExclusiveAccessBubble::StopWatchingMouse() {
  hide_timeout_.Stop();
  idle_timeout_.Stop();
  mouse_position_checker_.Stop();
}

bool ExclusiveAccessBubble::IsWatchingMouse() const {
  return mouse_position_checker_.IsRunning();
}

void ExclusiveAccessBubble::CheckMousePosition() {
  // Desired behavior (without the "simplified" flag):
  //
  // +------------+-----------------------------+------------+
  // | _  _  _  _ | Exit full screen mode (F11) | _  _  _  _ |  Slide-in region
  // | _  _  _  _ \_____________________________/ _  _  _  _ |  Neutral region
  // |                                                       |  Slide-out region
  // :                                                       :
  //
  // * If app is not active, we hide the popup.
  // * If the mouse is offscreen or in the slide-out region, we hide the popup.
  // * If the mouse goes idle, we hide the popup.
  // * If the mouse is in the slide-in-region and not idle, we show the popup.
  // * If the mouse is in the neutral region and not idle, and the popup is
  //   currently sliding out, we show it again.  This facilitates users
  //   correcting us if they try to mouse horizontally towards the popup and
  //   unintentionally drop too low.
  // * Otherwise, we do nothing, because the mouse is in the neutral region and
  //   either the popup is hidden or the mouse is not idle, so we don't want to
  //   change anything's state.
  //
  // With the "simplified" flag, we ignore all this and just show and hide based
  // on timers (not mouse position).

  gfx::Point cursor_pos = GetCursorScreenPoint();

  // Check to see whether the mouse is idle.
  if (cursor_pos != last_mouse_pos_) {
    // The mouse moved; reset the idle timer.
    idle_timeout_.Stop();  // If the timer isn't running, this is a no-op.
    idle_timeout_.Start(FROM_HERE,
                        base::TimeDelta::FromMilliseconds(kIdleTimeMs), this,
                        &ExclusiveAccessBubble::CheckMousePosition);
  }
  last_mouse_pos_ = cursor_pos;

  if (!IsWindowActive() || !WindowContainsPoint(cursor_pos) ||
      cursor_pos.y() >= GetPopupRect(true).bottom() ||
      !idle_timeout_.IsRunning()) {
    // Classic mode: The cursor is offscreen, in the slide-out region, or idle.
    // Simplified mode: Always come here (never check for mouse entering the top
    // of screen).
    if (!hide_timeout_.IsRunning())
      Hide();
  } else if (cursor_pos.y() < kSlideInRegionHeightPx &&
             CanMouseTriggerSlideIn()) {
    Show();
  } else if (IsAnimating()) {
    // The cursor is not idle and either it's in the slide-in region or it's in
    // the neutral region and we're sliding in or out.
    Show();
  }
}

void ExclusiveAccessBubble::ExitExclusiveAccess() {
}

void ExclusiveAccessBubble::Accept() {
}

void ExclusiveAccessBubble::Cancel() {
}

base::string16 ExclusiveAccessBubble::GetCurrentMessageText() const {
  return l10n_util::GetStringUTF16(IDS_FULLSCREEN_USER_ENTERED_FULLSCREEN);
}

void ExclusiveAccessBubble::ShowAndStartTimers() {
  Show();

  // Do not allow the notification to hide for a few seconds.
  hide_timeout_.Start(FROM_HERE,
                      base::TimeDelta::FromMilliseconds(kInitialDelayMs), this,
                      &ExclusiveAccessBubble::CheckMousePosition);

  // Do not show the notification again until a long time has elapsed.
  suppress_notify_timeout_.Start(
      FROM_HERE, base::TimeDelta::FromMilliseconds(kSnoozeNotificationsTimeMs),
      this, &ExclusiveAccessBubble::CheckMousePosition);
}

}  // namespace xwalk
