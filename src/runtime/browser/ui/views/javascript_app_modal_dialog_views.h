// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_UI_VIEWS_JAVASCRIPT_APP_MODAL_DIALOG_VIEWS_H_
#define CAMEO_SRC_RUNTIME_BROWSER_UI_VIEWS_JAVASCRIPT_APP_MODAL_DIALOG_VIEWS_H_

#include "base/memory/scoped_ptr.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/native_app_modal_dialog.h"
#include "ui/views/window/dialog_delegate.h"

class JavaScriptAppModalDialog;

namespace views {
class MessageBoxView;
}

class JavaScriptAppModalDialogViews : public NativeAppModalDialog,
                                      public views::DialogDelegate {
 public:
  explicit JavaScriptAppModalDialogViews(JavaScriptAppModalDialog* parent);
  virtual ~JavaScriptAppModalDialogViews();

  // Overridden from NativeAppModalDialog:
  virtual int GetAppModalDialogButtons() const OVERRIDE;
  virtual void ShowAppModalDialog() OVERRIDE;
  virtual void ActivateAppModalDialog() OVERRIDE;
  virtual void CloseAppModalDialog() OVERRIDE;
  virtual void AcceptAppModalDialog() OVERRIDE;
  virtual void CancelAppModalDialog() OVERRIDE;

  // Overridden from views::DialogDelegate:
  virtual int GetDefaultDialogButton() const OVERRIDE;
  virtual int GetDialogButtons() const OVERRIDE;
  virtual string16 GetWindowTitle() const OVERRIDE;
  virtual void WindowClosing() OVERRIDE;
  virtual void DeleteDelegate() OVERRIDE;
  virtual bool Cancel() OVERRIDE;
  virtual bool Accept() OVERRIDE;
  virtual string16 GetDialogButtonLabel(ui::DialogButton button) const OVERRIDE;

  // Overridden from views::WidgetDelegate:
  virtual ui::ModalType GetModalType() const OVERRIDE;
  virtual views::View* GetContentsView() OVERRIDE;
  virtual views::View* GetInitiallyFocusedView() OVERRIDE;
  virtual void OnClose() OVERRIDE;
  virtual views::Widget* GetWidget() OVERRIDE;
  virtual const views::Widget* GetWidget() const OVERRIDE;

 private:
  // A pointer to the AppModalDialog that owns us.
  scoped_ptr<JavaScriptAppModalDialog> parent_;

  // The message box view whose commands we handle.
  views::MessageBoxView* message_box_view_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptAppModalDialogViews);
};

#endif  // CAMEO_SRC_RUNTIME_BROWSER_UI_VIEWS_JAVASCRIPT_APP_MODAL_DIALOG_VIEWS_H_
