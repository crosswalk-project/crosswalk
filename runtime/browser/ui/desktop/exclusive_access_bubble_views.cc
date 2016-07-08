// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop/exclusive_access_bubble_views.h"

#include <algorithm>
#include <vector>

#include "base/i18n/case_conversion.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/notification_service.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/display/screen.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/animation/slide_animation.h"
#include "ui/gfx/canvas.h"
#include "ui/native_theme/native_theme.h"
#include "ui/strings/grit/ui_strings.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/link.h"
#include "ui/views/controls/link_listener.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "url/gurl.h"
#include "xwalk/grit/xwalk_resources.h"
#include "xwalk/runtime/browser/ui/desktop/exclusive_access_bubble_views_context.h"

#if defined(OS_WIN)
#include "ui/base/l10n/l10n_util_win.h"
#endif

namespace xwalk {

// ExclusiveAccessView ---------------------------------------------------------
namespace {

  // Space between the site info label and the buttons / link.
  const int kMiddlePaddingPx = 30;

  // Opacity of the background (out of 255).
  const unsigned char kBackgroundOpacity = 180;

}  // namespace

class ExclusiveAccessBubbleViews::ExclusiveAccessView
    : public views::View,
      public views::ButtonListener,
      public views::LinkListener {
 public:
  ExclusiveAccessView(ExclusiveAccessBubbleViews* bubble,
                      const base::string16& accelerator,
                      ExclusiveAccessBubble::Type bubble_type);
  ~ExclusiveAccessView() override;

  // views::ButtonListener
  void ButtonPressed(views::Button* sender, const ui::Event& event) override;

  // views::LinkListener
  void LinkClicked(views::Link* source, int event_flags) override;

  void UpdateContent(ExclusiveAccessBubble::Type bubble_type);

 private:
  ExclusiveAccessBubbleViews* bubble_;

  // Clickable hint text for exiting fullscreen mode. (Non-simplified mode
  // only.)
  views::Link* link_;
  // Informational label: 'www.foo.com has gone fullscreen'. (Non-simplified
  // mode only.)
  views::Label* message_label_;
  const base::string16 browser_fullscreen_exit_accelerator_;
  DISALLOW_COPY_AND_ASSIGN(ExclusiveAccessView);
};

ExclusiveAccessBubbleViews::ExclusiveAccessView::ExclusiveAccessView(
    ExclusiveAccessBubbleViews* bubble,
    const base::string16& accelerator,
    ExclusiveAccessBubble::Type bubble_type)
    : bubble_(bubble),
      link_(nullptr),
      message_label_(nullptr),
      browser_fullscreen_exit_accelerator_(accelerator) {
  views::BubbleBorder::Shadow shadow_type = views::BubbleBorder::BIG_SHADOW;
#if defined(OS_LINUX)
  // Use a smaller shadow on Linux (including ChromeOS) as the shadow assets can
  // overlap each other in a fullscreen notification bubble.
  // See http://crbug.com/462983.
  shadow_type = views::BubbleBorder::SMALL_SHADOW;
#endif

  SkColor background_color = SkColorSetA(SK_ColorBLACK, kBackgroundOpacity);
  SkColor foreground_color = SK_ColorWHITE;
  std::unique_ptr<views::BubbleBorder> bubble_border(new views::BubbleBorder(
      views::BubbleBorder::NONE, shadow_type, background_color));
  set_background(new views::BubbleBackground(bubble_border.get()));
  SetBorder(std::move(bubble_border));

  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  const gfx::FontList& medium_font_list =
      rb.GetFontList(ui::ResourceBundle::MediumFont);

  message_label_ = new views::Label(base::string16(), medium_font_list);
  message_label_->SetEnabledColor(foreground_color);
  message_label_->SetBackgroundColor(background_color);

  link_ = new views::Link();
  link_->SetFocusBehavior(FocusBehavior::NEVER);
  link_->set_listener(this);
  link_->SetFontList(medium_font_list);
  link_->SetPressedColor(foreground_color);
  link_->SetEnabledColor(foreground_color);
  link_->SetBackgroundColor(background_color);
  link_->SetVisible(false);

  DCHECK(message_label_);
  AddChildView(message_label_);
  AddChildView(link_);

  views::BoxLayout* layout = new views::BoxLayout(
      views::BoxLayout::kHorizontal, kPaddingPx, kPaddingPx, kMiddlePaddingPx);
  SetLayoutManager(layout);
  UpdateContent(bubble_type);
}

ExclusiveAccessBubbleViews::ExclusiveAccessView::~ExclusiveAccessView() {
}

void ExclusiveAccessBubbleViews::ExclusiveAccessView::ButtonPressed(
    views::Button* sender,
    const ui::Event& event) {
}

void ExclusiveAccessBubbleViews::ExclusiveAccessView::LinkClicked(
    views::Link* link,
    int event_flags) {
  bubble_->ExitExclusiveAccess();
}

void ExclusiveAccessBubbleViews::ExclusiveAccessView::UpdateContent(
    ExclusiveAccessBubble::Type bubble_type) {

  DCHECK(message_label_);
  message_label_->SetText(bubble_->GetCurrentMessageText());

  base::string16 link_text;

  switch (bubble_type) {
    case ExclusiveAccessBubble::TYPE_APPLICATION_FULLSCREEN_CLOSE_INSTRUCTION:
      link_text =
        l10n_util::GetStringUTF16(IDS_DISPLAY_MODE_FULLSCREEN_EXIT_APPLICATION);
      break;
    case ExclusiveAccessBubble::TYPE_BROWSER_FULLSCREEN_EXIT_INSTRUCTION:
      link_text = l10n_util::GetStringUTF16(IDS_FULLSCREEN_EXIT_MODE);
      break;
    default:
      link_text = base::string16();
  }

  link_->SetText(link_text);
  link_->SetVisible(true);
}

// ExclusiveAccessBubbleViews --------------------------------------------------

ExclusiveAccessBubbleViews::ExclusiveAccessBubbleViews(
    ExclusiveAccessBubbleViewsContext* context,
    ExclusiveAccessBubble::Type bubble_type,
    gfx::NativeWindow parent)
    : ExclusiveAccessBubble(bubble_type),
      bubble_view_context_(context),
      popup_(nullptr),
      animation_(new gfx::SlideAnimation(this)),
      animated_attribute_(ANIMATED_ATTRIBUTE_BOUNDS) {
  // With the simplified fullscreen UI flag, initially hide the bubble;
  // otherwise, initially show it.
  double initial_value = 1;
  animation_->Reset(initial_value);

  // Create the contents view.
  ui::Accelerator accelerator(ui::VKEY_UNKNOWN, ui::EF_NONE);
  bool got_accelerator = true;
  DCHECK(got_accelerator);
  view_ = new ExclusiveAccessView(this, accelerator.GetShortcutText(),
                                  bubble_type);

  // TODO(yzshen): Change to use the new views bubble, BubbleDelegateView.
  // TODO(pkotwicz): When this becomes a views bubble, make sure that this
  // bubble is ignored by ImmersiveModeControllerAsh::BubbleManager.
  // Initialize the popup.
  popup_ = new views::Widget;
  views::Widget::InitParams params(views::Widget::InitParams::TYPE_POPUP);
  params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
  params.ownership = views::Widget::InitParams::WIDGET_OWNS_NATIVE_WIDGET;
  params.parent = parent;
  // The simplified UI just shows a notice; clicks should go through to the
  // underlying window.
  params.accept_events = true;
  popup_->Init(params);
  popup_->SetContentsView(view_);
  gfx::Size size = GetPopupRect(true).size();
  popup_->SetBounds(GetPopupRect(false));
  // We set layout manager to nullptr to prevent the widget from sizing its
  // contents to the same size as itself. This prevents the widget contents from
  // shrinking while we animate the height of the popup to give the impression
  // that it is sliding off the top of the screen.
  popup_->GetRootView()->SetLayoutManager(nullptr);
  view_->SetBounds(0, 0, size.width(), size.height());
  popup_->ShowInactive();  // This does not activate the popup.
  popup_->AddObserver(this);
  UpdateForImmersiveState();
}

ExclusiveAccessBubbleViews::~ExclusiveAccessBubbleViews() {
  popup_->RemoveObserver(this);

  // This is tricky.  We may be in an ATL message handler stack, in which case
  // the popup cannot be deleted yet.  We also can't set the popup's ownership
  // model to NATIVE_WIDGET_OWNS_WIDGET because if the user closed the last tab
  // while in fullscreen mode, Windows has already destroyed the popup HWND by
  // the time we get here, and thus either the popup will already have been
  // deleted (if we set this in our constructor) or the popup will never get
  // another OnFinalMessage() call (if not, as currently).  So instead, we tell
  // the popup to synchronously hide, and then asynchronously close and delete
  // itself.
  popup_->Close();
  base::MessageLoop::current()->DeleteSoon(FROM_HERE, popup_);
}

void ExclusiveAccessBubbleViews::UpdateContent(
  ExclusiveAccessBubble::Type bubble_type) {
  if (bubble_type_ == bubble_type)
    return;

  bubble_type_ = bubble_type;
  view_->UpdateContent(bubble_type);

  gfx::Size size = GetPopupRect(true).size();
  view_->SetSize(size);
  popup_->SetBounds(GetPopupRect(false));
  Show();

  // Stop watching the mouse even if UpdateMouseWatcher() will start watching
  // it again so that the popup with the new content is visible for at least
  // |kInitialDelayMs|.
  StopWatchingMouse();

  UpdateMouseWatcher();
}

void ExclusiveAccessBubbleViews::RepositionIfVisible() {
  if (popup_->IsVisible())
    UpdateBounds();
}

views::View* ExclusiveAccessBubbleViews::GetView() {
  return view_;
}

void ExclusiveAccessBubbleViews::UpdateMouseWatcher() {
  bool should_watch_mouse = false;
  if (popup_->IsVisible())
    should_watch_mouse = true;
  else
    should_watch_mouse = CanMouseTriggerSlideIn();

  if (should_watch_mouse == IsWatchingMouse())
    return;

  if (should_watch_mouse)
    StartWatchingMouse();
  else
    StopWatchingMouse();
}

void ExclusiveAccessBubbleViews::UpdateForImmersiveState() {
  AnimatedAttribute expected_animated_attribute =
              bubble_view_context_->IsImmersiveModeEnabled()
          ? ANIMATED_ATTRIBUTE_OPACITY
          : ANIMATED_ATTRIBUTE_BOUNDS;
  if (animated_attribute_ != expected_animated_attribute) {
    // If an animation is currently in progress, skip to the end because
    // switching the animated attribute midway through the animation looks
    // weird.
    animation_->End();

    animated_attribute_ = expected_animated_attribute;

    // We may have finished hiding |popup_|. However, the bounds animation
    // assumes |popup_| has the opacity when it is fully shown and the opacity
    // animation assumes |popup_| has the bounds when |popup_| is fully shown.
    if (animated_attribute_ == ANIMATED_ATTRIBUTE_BOUNDS)
      popup_->SetOpacity(255);
    else
      UpdateBounds();
  }

  UpdateMouseWatcher();
}

void ExclusiveAccessBubbleViews::UpdateBounds() {
  gfx::Rect popup_rect(GetPopupRect(false));
  if (!popup_rect.IsEmpty()) {
    popup_->SetBounds(popup_rect);
    view_->SetY(popup_rect.height() - view_->height());
  }
}

views::View* ExclusiveAccessBubbleViews::GetBrowserRootView() const {
  return bubble_view_context_->GetBubbleAssociatedWidget()->GetRootView();
}

void ExclusiveAccessBubbleViews::AnimationProgressed(
    const gfx::Animation* animation) {
  if (animated_attribute_ == ANIMATED_ATTRIBUTE_OPACITY) {
    int opacity = animation_->CurrentValueBetween(0, 255);
    if (opacity == 0) {
      popup_->Hide();
    } else {
      popup_->Show();
      popup_->SetOpacity(opacity);
    }
  } else {
    if (GetPopupRect(false).IsEmpty()) {
      popup_->Hide();
    } else {
      UpdateBounds();
      popup_->Show();
    }
  }
}

void ExclusiveAccessBubbleViews::AnimationEnded(
    const gfx::Animation* animation) {
  AnimationProgressed(animation);
}

gfx::Rect ExclusiveAccessBubbleViews::GetPopupRect(
    bool ignore_animation_state) const {
  gfx::Size size(view_->GetPreferredSize());
  // NOTE: don't use the bounds of the root_view_. On linux GTK changing window
  // size is async. Instead we use the size of the screen.
  gfx::Rect widget_bounds = bubble_view_context_->GetBubbleAssociatedWidget()
                                ->GetClientAreaBoundsInScreen();
  int x = widget_bounds.x() + (widget_bounds.width() - size.width()) / 2;

  int top_container_bottom = widget_bounds.y();
  if (bubble_view_context_->IsImmersiveModeEnabled()) {
    // Skip querying the top container height in non-immersive fullscreen
    // because:
    // - The top container height is always zero in non-immersive fullscreen.
    // - Querying the top container height may return the height before entering
    //   fullscreen because layout is disabled while entering fullscreen.
    // A visual glitch due to the delayed layout is avoided in immersive
    // fullscreen because entering fullscreen starts with the top container
    // revealed. When revealed, the top container has the same height as before
    // entering fullscreen.
    top_container_bottom =
        bubble_view_context_->GetTopContainerBoundsInScreen().bottom();
  }
  // |desired_top| is the top of the bubble area including the shadow.
  int desired_top = kPopupTopPx - view_->border()->GetInsets().top();
  int y = top_container_bottom + desired_top;

  if (!ignore_animation_state &&
      animated_attribute_ == ANIMATED_ATTRIBUTE_BOUNDS) {
    int total_height = size.height() + desired_top;
    int popup_bottom = animation_->CurrentValueBetween(total_height, 0);
    int y_offset = std::min(popup_bottom, desired_top);
    size.set_height(size.height() - popup_bottom + y_offset);
    y -= y_offset;
  }
  return gfx::Rect(gfx::Point(x, y), size);
}

gfx::Point ExclusiveAccessBubbleViews::GetCursorScreenPoint() {
  gfx::Point cursor_pos =
      display::Screen::GetScreen()->GetCursorScreenPoint();
  views::View::ConvertPointFromScreen(GetBrowserRootView(), &cursor_pos);
  return cursor_pos;
}

bool ExclusiveAccessBubbleViews::WindowContainsPoint(gfx::Point pos) {
  return GetBrowserRootView()->HitTestPoint(pos);
}

bool ExclusiveAccessBubbleViews::IsWindowActive() {
  return bubble_view_context_->GetBubbleAssociatedWidget()->IsActive();
}

void ExclusiveAccessBubbleViews::Hide() {
  animation_->SetSlideDuration(kSlideOutDurationMs);
  animation_->Hide();
}

void ExclusiveAccessBubbleViews::Show() {
  animation_->SetSlideDuration(kSlideInDurationMs);
  animation_->Show();
}

bool ExclusiveAccessBubbleViews::IsAnimating() {
  return animation_->is_animating();
}

bool ExclusiveAccessBubbleViews::CanMouseTriggerSlideIn() const {
  return !bubble_view_context_->IsImmersiveModeEnabled();
}

void ExclusiveAccessBubbleViews::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  UpdateForImmersiveState();
}

void ExclusiveAccessBubbleViews::OnWidgetVisibilityChanged(
    views::Widget* widget,
    bool visible) {
  UpdateMouseWatcher();
}

void ExclusiveAccessBubbleViews::ExitExclusiveAccess() {
  switch (bubble_type_) {
    case ExclusiveAccessBubble::TYPE_APPLICATION_FULLSCREEN_CLOSE_INSTRUCTION:
      bubble_view_context_->GetNativeAppViews()->ExitApplication();
      break;
    case ExclusiveAccessBubble::TYPE_BROWSER_FULLSCREEN_EXIT_INSTRUCTION:
      bubble_view_context_->GetNativeAppViews()->SetFullscreen(false);
      break;
    default:
      break;
  }
}

}  // namespace xwalk
