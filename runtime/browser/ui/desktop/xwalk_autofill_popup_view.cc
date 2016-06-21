// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop/xwalk_autofill_popup_view.h"

#include "base/threading/thread_task_runner_handle.h"
#include "components/autofill/core/browser/popup_item_ids.h"
#include "components/autofill/core/browser/suggestion.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/text_utils.h"
#include "ui/views/border.h"
#include "ui/views/widget/widget.h"
#include "xwalk/runtime/browser/ui/desktop/xwalk_autofill_popup_controller.h"

namespace xwalk {

const SkColor XWalkAutofillPopupView::kBorderColor =
    SkColorSetARGB(0xFF, 0xC7, 0xCA, 0xCE);
const SkColor XWalkAutofillPopupView::kHoveredBackgroundColor =
    SkColorSetARGB(0xFF, 0xCD, 0xCD, 0xCD);
const SkColor XWalkAutofillPopupView::kItemTextColor =
    SkColorSetARGB(0xFF, 0x7F, 0x7F, 0x7F);
const SkColor XWalkAutofillPopupView::kPopupBackground =
    SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF);
const SkColor XWalkAutofillPopupView::kValueTextColor =
    SkColorSetARGB(0xFF, 0x00, 0x00, 0x00);
const SkColor XWalkAutofillPopupView::kWarningTextColor =
    SkColorSetARGB(0xFF, 0x7F, 0x7F, 0x7F);

XWalkAutofillPopupView::XWalkAutofillPopupView(
    XWalkAutofillPopupController* controller,
    views::Widget* parent_widget)
    : controller_(controller),
    parent_widget_(parent_widget),
    weak_ptr_factory_(this) {
}

XWalkAutofillPopupView::~XWalkAutofillPopupView() {
  if (controller_) {
    controller_->ViewDestroyed();
    RemoveObserver();
  }
}

void XWalkAutofillPopupView::Show() {
  const bool initialize_widget = !GetWidget();
  if (initialize_widget) {
    parent_widget_->AddObserver(this);
    views::FocusManager* focus_manager = parent_widget_->GetFocusManager();
    focus_manager->RegisterAccelerator(
        ui::Accelerator(ui::VKEY_RETURN, ui::EF_NONE),
        ui::AcceleratorManager::kNormalPriority,
        this);
    focus_manager->RegisterAccelerator(
        ui::Accelerator(ui::VKEY_ESCAPE, ui::EF_NONE),
        ui::AcceleratorManager::kNormalPriority,
        this);

    // The widget is destroyed by the corresponding NativeWidget, so we use
    // a weak pointer to hold the reference and don't have to worry about
    // deletion.
    views::Widget* widget = new views::Widget;
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_POPUP);
    params.delegate = this;
    params.parent = parent_widget_->GetNativeView();
    widget->Init(params);
    widget->SetContentsView(this);

    // No animation for popup appearance (too distracting).
    widget->SetVisibilityAnimationTransition(views::Widget::ANIMATE_HIDE);

    show_time_ = base::Time::Now();
  }

  SetBorder(views::Border::CreateSolidBorder(kPopupBorderThickness,
    kBorderColor));

  UpdateBoundsAndRedrawPopup();
  GetWidget()->Show();

  // Showing the widget can change native focus (which would result in an
  // immediate hiding of the popup). Only start observing after shown.
  if (initialize_widget)
    views::WidgetFocusManager::GetInstance()->AddFocusChangeListener(this);
}

void XWalkAutofillPopupView::Hide() {
  // The controller is no longer valid after it hides us.
  controller_ = nullptr;

  RemoveObserver();

  if (GetWidget()) {
    // Don't call CloseNow() because some of the functions higher up the stack
    // assume the the widget is still valid after this point.
    // http://crbug.com/229224
    // NOTE: This deletes |this|.
    GetWidget()->Close();
  } else {
    delete this;
  }
}

void XWalkAutofillPopupView::OnWidgetBoundsChanged(views::Widget* widget,
    const gfx::Rect& new_bounds) {
  DCHECK_EQ(widget, parent_widget_);
  HideController();
}

void XWalkAutofillPopupView::RemoveObserver() {
  parent_widget_->GetFocusManager()->UnregisterAccelerators(this);
  parent_widget_->RemoveObserver(this);
  views::WidgetFocusManager::GetInstance()->RemoveFocusChangeListener(this);
}

void XWalkAutofillPopupView::OnNativeFocusChanged(gfx::NativeView focused_now) {
  if (GetWidget() && GetWidget()->GetNativeView() != focused_now)
    HideController();
}

void XWalkAutofillPopupView::OnMouseCaptureLost() {
  ClearSelection();
}

void XWalkAutofillPopupView::UpdateBoundsAndRedrawPopup() {
  GetWidget()->SetBounds(controller_->popup_bounds());
  SchedulePaint();
}

bool XWalkAutofillPopupView::OnMouseDragged(const ui::MouseEvent& event) {
  if (HitTestPoint(event.location())) {
    SetSelection(event.location());

    // We must return true in order to get future OnMouseDragged and
    // OnMouseReleased events.
    return true;
  }

  // If we move off of the popup, we lose the selection.
  ClearSelection();
  return false;
}

void XWalkAutofillPopupView::OnMouseExited(const ui::MouseEvent& event) {
  // Pressing return causes the cursor to hide, which will generate an
  // OnMouseExited event. Pressing return should activate the current selection
  // via AcceleratorPressed, so we need to let that run first.
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::Bind(&XWalkAutofillPopupView::ClearSelection,
      weak_ptr_factory_.GetWeakPtr()));
}

void XWalkAutofillPopupView::OnMouseMoved(const ui::MouseEvent& event) {
  // A synthesized mouse move will be sent when the popup is first shown.
  // Don't preview a suggestion if the mouse happens to be hovering there.
#if defined(OS_WIN)
  // TODO(rouslan): Use event.time_stamp() and ui::EventTimeForNow() when they
  // become comparable. http://crbug.com/453559
  if (base::Time::Now() - show_time_ <= base::TimeDelta::FromMilliseconds(50))
    return;
#else
  if (event.flags() & ui::EF_IS_SYNTHESIZED)
    return;
#endif

  if (HitTestPoint(event.location()))
    SetSelection(event.location());
  else
    ClearSelection();
}

bool XWalkAutofillPopupView::OnMousePressed(const ui::MouseEvent& event) {
  return event.GetClickCount() == 1;
}

void XWalkAutofillPopupView::OnMouseReleased(const ui::MouseEvent& event) {
  // We only care about the left click.
  if (event.IsOnlyLeftMouseButton() && HitTestPoint(event.location()))
    AcceptSelection(event.location());
}

void XWalkAutofillPopupView::OnGestureEvent(ui::GestureEvent* event) {
  switch (event->type()) {
  case ui::ET_GESTURE_TAP_DOWN:
  case ui::ET_GESTURE_SCROLL_BEGIN:
  case ui::ET_GESTURE_SCROLL_UPDATE:
    if (HitTestPoint(event->location()))
      SetSelection(event->location());
    else
      ClearSelection();
    break;
  case ui::ET_GESTURE_TAP:
  case ui::ET_GESTURE_SCROLL_END:
    if (HitTestPoint(event->location()))
      AcceptSelection(event->location());
    else
      ClearSelection();
    break;
  case ui::ET_GESTURE_TAP_CANCEL:
  case ui::ET_SCROLL_FLING_START:
    ClearSelection();
    break;
  default:
    return;
  }
  event->SetHandled();
}

bool XWalkAutofillPopupView::AcceleratorPressed(
    const ui::Accelerator& accelerator) {
  DCHECK_EQ(accelerator.modifiers(), ui::EF_NONE);

  if (accelerator.key_code() == ui::VKEY_ESCAPE) {
    HideController();
    return true;
  }

  if (accelerator.key_code() == ui::VKEY_RETURN)
    return controller_->AcceptSelectedLine();

  NOTREACHED();
  return false;
}

void XWalkAutofillPopupView::SetSelection(const gfx::Point& point) {
  if (controller_)
    controller_->SetSelectionAtPoint(point);
}

void XWalkAutofillPopupView::AcceptSelection(const gfx::Point& point) {
  if (!controller_)
    return;

  controller_->SetSelectionAtPoint(point);
  controller_->AcceptSelectedLine();
}

void XWalkAutofillPopupView::ClearSelection() {
  if (controller_)
    controller_->SelectionCleared();
}

void XWalkAutofillPopupView::HideController() {
  if (controller_)
    controller_->Hide();
}

gfx::NativeView XWalkAutofillPopupView::container_view() {
  return controller_->container_view();
}

void XWalkAutofillPopupView::OnPaint(gfx::Canvas* canvas) {
  if (!controller_)
    return;

  canvas->DrawColor(kPopupBackground);
  OnPaintBorder(canvas);

  for (size_t i = 0; i < controller_->GetLineCount(); ++i) {
    gfx::Rect line_rect = controller_->GetRowBounds(i);

    if (controller_->GetSuggestionAt(i).frontend_id ==
      autofill::POPUP_ITEM_ID_SEPARATOR) {
      canvas->FillRect(line_rect, kItemTextColor);
    } else {
      DrawAutofillEntry(canvas, i, line_rect);
    }
  }
}

void XWalkAutofillPopupView::InvalidateRow(size_t row) {
  SchedulePaintInRect(controller_->GetRowBounds(row));
}

void XWalkAutofillPopupView::DrawAutofillEntry(gfx::Canvas* canvas,
    int index,
    const gfx::Rect& entry_rect) {
  if (controller_->selected_line() == index)
    canvas->FillRect(entry_rect, kHoveredBackgroundColor);

  const bool is_rtl = controller_->IsRTL();
  const int text_align =
    is_rtl ? gfx::Canvas::TEXT_ALIGN_RIGHT : gfx::Canvas::TEXT_ALIGN_LEFT;
  gfx::Rect value_rect = entry_rect;
  value_rect.Inset(kEndPadding, 0);
  canvas->DrawStringRectWithFlags(
      controller_->GetElidedValueAt(index),
      controller_->GetValueFontListForRow(index),
      controller_->IsWarning(index) ? kWarningTextColor : kValueTextColor,
      value_rect, text_align);

  // Use this to figure out where all the other Autofill items should be placed.
  int x_align_left = is_rtl ? kEndPadding : entry_rect.right() - kEndPadding;

  // Draw the Autofill icon, if one exists
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  int row_height = controller_->GetRowBounds(index).height();
  if (!controller_->GetSuggestionAt(index).icon.empty()) {
    int icon = controller_->GetIconResourceID(
        controller_->GetSuggestionAt(index).icon);
    DCHECK_NE(-1, icon);
    const gfx::ImageSkia* image = rb.GetImageSkiaNamed(icon);
    int icon_y = entry_rect.y() + (row_height - image->height()) / 2;

    x_align_left += is_rtl ? 0 : -image->width();

    canvas->DrawImageInt(*image, x_align_left, icon_y);

    x_align_left += is_rtl ? image->width() + kIconPadding : -kIconPadding;
  }

  // Draw the label text.
  const int label_width =
      gfx::GetStringWidth(controller_->GetElidedLabelAt(index),
      controller_->GetLabelFontList());
  if (!is_rtl)
    x_align_left -= label_width;

  canvas->DrawStringRectWithFlags(
      controller_->GetElidedLabelAt(index), controller_->GetLabelFontList(),
      kItemTextColor,
      gfx::Rect(x_align_left, entry_rect.y(), label_width, entry_rect.height()),
      text_align);
}

XWalkAutofillPopupView* XWalkAutofillPopupView::Create(
    XWalkAutofillPopupController* controller) {
  views::Widget* observing_widget =
      views::Widget::GetTopLevelWidgetForNativeView(
          controller->container_view());

  // If the top level widget can't be found, cancel the popup since we can't
  // fully set it up.
  if (!observing_widget)
    return nullptr;

  return new XWalkAutofillPopupView(controller, observing_widget);
}

}  // namespace xwalk
