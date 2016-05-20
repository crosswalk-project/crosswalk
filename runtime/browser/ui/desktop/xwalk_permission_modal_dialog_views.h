// Copyright 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_MODAL_DIALOG_VIEWS_H_
#define XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_MODAL_DIALOG_VIEWS_H_

#include <memory>

#include "components/app_modal/native_app_modal_dialog.h"
#include "ui/views/window/dialog_delegate.h"

namespace views {
class MessageBoxView;
}

namespace xwalk {

class XWalkPermissionModalDialog;

class XWalkPermissionModalDialogViews : public app_modal::NativeAppModalDialog,
  public views::DialogDelegate {
 public:
  explicit XWalkPermissionModalDialogViews(XWalkPermissionModalDialog* parent);
  ~XWalkPermissionModalDialogViews() override;

  int GetAppModalDialogButtons() const override;
  void ShowAppModalDialog() override;
  void ActivateAppModalDialog() override;
  void CloseAppModalDialog() override;
  void AcceptAppModalDialog() override;
  void CancelAppModalDialog() override;
  bool IsShowing() const override;

  int GetDefaultDialogButton() const override;
  int GetDialogButtons() const override;
  base::string16 GetWindowTitle() const override;
  void DeleteDelegate() override;
  bool Cancel() override;
  bool Accept() override;
  base::string16 GetDialogButtonLabel(ui::DialogButton button) const override;

  ui::ModalType GetModalType() const override;
  views::View* GetContentsView() override;
  views::View* GetInitiallyFocusedView() override;
  views::Widget* GetWidget() override;
  const views::Widget* GetWidget() const override;

 private:
  // A pointer to the AppModalDialog that owns us.
  std::unique_ptr<XWalkPermissionModalDialog> parent_;

  // The message box view whose commands we handle.
  std::unique_ptr<views::MessageBoxView> message_box_view_;

  DISALLOW_COPY_AND_ASSIGN(XWalkPermissionModalDialogViews);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_MODAL_DIALOG_VIEWS_H_
