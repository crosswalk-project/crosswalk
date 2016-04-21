// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_modal_dialog_views.h"

#include "base/strings/utf_string_conversions.h"
#include "components/app_modal/javascript_app_modal_dialog.h"
#include "components/constrained_window/constrained_window_views.h"
#include "grit/components_strings.h"
#include "grit/xwalk_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/views/controls/message_box_view.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/dialog_client_view.h"
#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_modal_dialog.h"

namespace xwalk {

XWalkPermissionModalDialogViews::XWalkPermissionModalDialogViews(
    XWalkPermissionModalDialog* parent)
  : parent_(parent) {
  int options = views::MessageBoxView::DETECT_DIRECTIONALITY;

  views::MessageBoxView::InitParams params(parent->message_text());
  params.options = options;
  message_box_view_.reset(new views::MessageBoxView(params));
  message_box_view_->AddAccelerator(
      ui::Accelerator(ui::VKEY_C, ui::EF_CONTROL_DOWN));
}

XWalkPermissionModalDialogViews::~XWalkPermissionModalDialogViews() {
}

int XWalkPermissionModalDialogViews::GetAppModalDialogButtons() const {
  return GetDialogButtons();
}

void XWalkPermissionModalDialogViews::ShowAppModalDialog() {
  GetWidget()->Show();
}

void XWalkPermissionModalDialogViews::ActivateAppModalDialog() {
  GetWidget()->Show();
  GetWidget()->Activate();
}

void XWalkPermissionModalDialogViews::CloseAppModalDialog() {
  GetWidget()->Close();
}

void XWalkPermissionModalDialogViews::AcceptAppModalDialog() {
  GetDialogClientView()->AcceptWindow();
}

void XWalkPermissionModalDialogViews::CancelAppModalDialog() {
  GetDialogClientView()->CancelWindow();
}

bool XWalkPermissionModalDialogViews::IsShowing() const {
  return GetWidget()->IsVisible();
}

int XWalkPermissionModalDialogViews::GetDefaultDialogButton() const {
  return ui::DIALOG_BUTTON_OK;
}

int XWalkPermissionModalDialogViews::GetDialogButtons() const {
  return ui::DIALOG_BUTTON_OK | ui::DIALOG_BUTTON_CANCEL;
}

base::string16 XWalkPermissionModalDialogViews::GetWindowTitle() const {
  return parent_->title();
}

void XWalkPermissionModalDialogViews::DeleteDelegate() {
  delete this;
}

bool XWalkPermissionModalDialogViews::Cancel() {
  parent_->OnCancel(message_box_view_->IsCheckBoxSelected());
  return true;
}

bool XWalkPermissionModalDialogViews::Accept() {
  parent_->OnAccept(message_box_view_->GetInputText(),
      message_box_view_->IsCheckBoxSelected());
  return true;
}

views::Widget* XWalkPermissionModalDialogViews::GetWidget() {
  return message_box_view_->GetWidget();
}

const views::Widget* XWalkPermissionModalDialogViews::GetWidget() const {
  return message_box_view_->GetWidget();
}

base::string16 XWalkPermissionModalDialogViews::GetDialogButtonLabel(
    ui::DialogButton button) const {
  if (button == ui::DIALOG_BUTTON_OK) {
    return l10n_util::GetStringUTF16(IDS_PERMISSION_ALLOW);
  } else if (button == ui::DIALOG_BUTTON_CANCEL) {
    return l10n_util::GetStringUTF16(IDS_PERMISSION_DENY);
  } else {
    return DialogDelegate::GetDialogButtonLabel(button);
  }
}

ui::ModalType XWalkPermissionModalDialogViews::GetModalType() const {
  return ui::MODAL_TYPE_SYSTEM;
}

views::View* XWalkPermissionModalDialogViews::GetContentsView() {
  return message_box_view_.get();
}

views::View* XWalkPermissionModalDialogViews::GetInitiallyFocusedView() {
  if (message_box_view_->text_box())
    return message_box_view_->text_box();
  return views::DialogDelegate::GetInitiallyFocusedView();
}

}  // namespace xwalk
