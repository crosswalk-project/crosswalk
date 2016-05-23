// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_DESKTOP_EXCLUSIVE_ACCESS_BUBBLE_VIEWS_H_
#define XWALK_RUNTIME_BROWSER_UI_DESKTOP_EXCLUSIVE_ACCESS_BUBBLE_VIEWS_H_

#include <memory>

#include "base/compiler_specific.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "ui/views/widget/widget_observer.h"
#include "xwalk/runtime/browser/ui/desktop/exclusive_access_bubble.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"

class GURL;
namespace gfx {
class SlideAnimation;
}
namespace views {
class View;
class Widget;
}

namespace xwalk {

class ExclusiveAccessBubbleViewsContext;

// ExclusiveAccessBubbleViews is responsible for showing a bubble atop the
// screen in fullscreen/mouse lock mode, telling users how to exit and providing
// a click target. The bubble auto-hides, and re-shows when the user moves to
// the screen top.
class ExclusiveAccessBubbleViews : public ExclusiveAccessBubble,
                                   public content::NotificationObserver,
                                   public views::WidgetObserver {
 public:
  ExclusiveAccessBubbleViews(ExclusiveAccessBubbleViewsContext* context,
    ExclusiveAccessBubble::Type bubble_type, gfx::NativeWindow parent);
  ~ExclusiveAccessBubbleViews() override;

  void UpdateContent(ExclusiveAccessBubble::Type
      bubble_type);

  // Repositions |popup_| if it is visible.
  void RepositionIfVisible();

  views::View* GetView();

 private:
  class ExclusiveAccessView;

  enum AnimatedAttribute {
    ANIMATED_ATTRIBUTE_BOUNDS,
    ANIMATED_ATTRIBUTE_OPACITY
  };

  // Starts or stops polling the mouse location based on |popup_| and
  // |bubble_type_|.
  void UpdateMouseWatcher();

  // Updates any state which depends on whether the user is in immersive
  // fullscreen.
  void UpdateForImmersiveState();

  // Updates |popup|'s bounds given |animation_| and |animated_attribute_|.
  void UpdateBounds();

  // Returns the root view containing |browser_view_|.
  views::View* GetBrowserRootView() const;

  // FullScreenExitBubble overrides:
  void AnimationProgressed(const gfx::Animation* animation) override;
  void AnimationEnded(const gfx::Animation* animation) override;
  gfx::Rect GetPopupRect(bool ignore_animation_state) const override;
  gfx::Point GetCursorScreenPoint() override;
  bool WindowContainsPoint(gfx::Point pos) override;
  bool IsWindowActive() override;
  void Hide() override;
  void Show() override;
  bool IsAnimating() override;
  bool CanMouseTriggerSlideIn() const override;
  void ExitExclusiveAccess() override;

  // content::NotificationObserver override:
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // views::WidgetObserver override:
  void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override;

  ExclusiveAccessBubbleViewsContext* const bubble_view_context_;

  views::Widget* popup_;

  // Animation controlling showing/hiding of the exit bubble.
  std::unique_ptr<gfx::SlideAnimation> animation_;

  // Attribute animated by |animation_|.
  AnimatedAttribute animated_attribute_;

  // The contents of the popup.
  ExclusiveAccessView* view_;

  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(ExclusiveAccessBubbleViews);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_DESKTOP_EXCLUSIVE_ACCESS_BUBBLE_VIEWS_H_
