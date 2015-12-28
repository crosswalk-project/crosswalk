// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_AUTOFILL_POPUP_VIEW_H_
#define XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_AUTOFILL_POPUP_VIEW_H_

#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "ui/views/focus/widget_focus_manager.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"

namespace xwalk {

class XWalkAutofillPopupController;

// Views toolkit implementation for a popup view.
class XWalkAutofillPopupView : public views::WidgetDelegateView,
    public views::WidgetFocusChangeListener,
    public views::WidgetObserver {
 public:
  static const size_t kPopupBorderThickness = 1;

  // The minimum amount of padding between the Autofill name and subtext,
  // in pixels.
  static const size_t kNamePadding = 15;

  // The amount of padding between icons in pixels.
  static const int kIconPadding = 5;

  // The amount of padding at the end of the popup in pixels.
  static const int kEndPadding = 3;

  // Height of the delete icon in pixels.
  static const int kDeleteIconHeight = 16;

  // Width of the delete icon in pixels.
  static const int kDeleteIconWidth = 16;

  static const SkColor kBorderColor;
  static const SkColor kHoveredBackgroundColor;
  static const SkColor kItemTextColor;
  static const SkColor kPopupBackground;
  static const SkColor kValueTextColor;
  static const SkColor kWarningTextColor;

  XWalkAutofillPopupView(XWalkAutofillPopupController* controller,
      views::Widget* parent_widget);

  static XWalkAutofillPopupView* Create(
      XWalkAutofillPopupController* controller);
  void Show();
  void Hide();
  void InvalidateRow(size_t row);
  void UpdateBoundsAndRedrawPopup();

 private:
  ~XWalkAutofillPopupView() override;

  // views::Views implementation.
  void OnMouseCaptureLost() override;
  bool OnMouseDragged(const ui::MouseEvent& event) override;
  void OnMouseExited(const ui::MouseEvent& event) override;
  void OnMouseMoved(const ui::MouseEvent& event) override;
  bool OnMousePressed(const ui::MouseEvent& event) override;
  void OnMouseReleased(const ui::MouseEvent& event) override;
  void OnGestureEvent(ui::GestureEvent* event) override;
  bool AcceleratorPressed(const ui::Accelerator& accelerator) override;

  // views::WidgetFocusChangeListener implementation.
  void OnNativeFocusChanged(gfx::NativeView focused_now) override;

  // views::WidgetObserver implementation.
  void OnWidgetBoundsChanged(views::Widget* widget,
    const gfx::Rect& new_bounds) override;

  // views::Views implementation
  void OnPaint(gfx::Canvas* canvas) override;

  // Draw the given autofill entry in |entry_rect|.
  void DrawAutofillEntry(gfx::Canvas* canvas,
    int index,
    const gfx::Rect& entry_rect);

  // Stop observing the widget.
  void RemoveObserver();

  void SetSelection(const gfx::Point& point);
  void AcceptSelection(const gfx::Point& point);
  void ClearSelection();

  // Hide the controller of this view. This assumes that doing so will
  // eventually hide this view in the process.
  void HideController();

  // Must return the container view for this popup.
  gfx::NativeView container_view();

  XWalkAutofillPopupController* controller_;

  // The widget of the window that triggered this popup.
  views::Widget* parent_widget_;

  // The time when the popup was shown.
  base::Time show_time_;

  base::WeakPtrFactory<XWalkAutofillPopupView> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(XWalkAutofillPopupView);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_AUTOFILL_POPUP_VIEW_H_
