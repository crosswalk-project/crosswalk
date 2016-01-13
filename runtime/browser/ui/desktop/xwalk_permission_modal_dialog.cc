// Copyright 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_modal_dialog.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "components/constrained_window/constrained_window_views.h"
#include "components/constrained_window/constrained_window_views_client.h"
#include "ui/gfx/text_elider.h"
#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_dialog_manager.h"
#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_modal_dialog_views.h"

#if defined(USE_AURA)
#include "ui/aura/window.h"
#endif

namespace xwalk {
namespace {

// Control maximum sizes of various texts passed to us from javascript.
#if defined(OS_POSIX) && !defined(OS_MACOSX)
// Two-dimensional eliding.  Reformat the text of the message dialog
// inserting line breaks because otherwise a single long line can overflow
// the message dialog (and crash/hang the GTK, depending on the version).
const int kMessageTextMaxRows = 32;
const int kMessageTextMaxCols = 132;
void EnforceMaxTextSize(const base::string16& in_string,
    base::string16* out_string) {
  gfx::ElideRectangleString(in_string, kMessageTextMaxRows,
    kMessageTextMaxCols, false, out_string);
}
#else
// One-dimensional eliding.  Trust the window system to break the string
// appropriately, but limit its overall length to something reasonable.
const int kMessageTextMaxSize = 3000;
void EnforceMaxTextSize(const base::string16& in_string,
    base::string16* out_string) {
  gfx::ElideString(in_string, kMessageTextMaxSize, out_string);
}
#endif

}  // namespace

XWalkPermissionModalDialog::XWalkPermissionModalDialog(
    content::WebContents* web_contents,
    const base::string16& message_text,
    const base::Callback<void(bool)>& callback)
    : app_modal::AppModalDialog(web_contents, base::string16()),
    callback_(callback) {
  EnforceMaxTextSize(message_text, &message_text_);
}

XWalkPermissionModalDialog::~XWalkPermissionModalDialog() {
}

app_modal::NativeAppModalDialog*
    XWalkPermissionModalDialog::CreateNativeDialog() {
  XWalkPermissionModalDialogViews* dialogViews =
      new XWalkPermissionModalDialogViews(this);

  web_contents()->GetDelegate()->ActivateContents(web_contents());
  gfx::NativeWindow parent_window =
      web_contents()->GetTopLevelNativeWindow();
#if defined(USE_AURA)
    if (!parent_window->GetRootWindow()) {
      // When we are part of a WebContents that isn't actually being displayed
      // on the screen, we can't actually attach to it.
      parent_window = nullptr;
    }
#endif
  constrained_window::CreateBrowserModalDialogViews(dialogViews, parent_window);
  return dialogViews;
}

void XWalkPermissionModalDialog::Invalidate() {
  if (!IsValid())
    return;

  AppModalDialog::Invalidate();
  if (!callback_.is_null()) {
    callback_.Run(false);
    callback_.Reset();
  }
  if (native_dialog())
    CloseModalDialog();
}

void XWalkPermissionModalDialog::OnCancel(bool suppress_js_messages) {
  // We need to do this before WM_DESTROY (WindowClosing()) as any parent frame
  // will receive its activation messages before this dialog receives
  // WM_DESTROY. The parent frame would then try to activate any modal dialogs
  // that were still open in the ModalDialogQueue, which would send activation
  // back to this one. The framework should be improved to handle this, so this
  // is a temporary workaround.
  CompleteDialog();

  NotifyDelegate(false);
}

void XWalkPermissionModalDialog::OnAccept(const base::string16& prompt_text,
    bool suppress_js_messages) {
  CompleteDialog();
  NotifyDelegate(true);
}

void XWalkPermissionModalDialog::OnClose() {
  NotifyDelegate(false);
}

void XWalkPermissionModalDialog::NotifyDelegate(bool success) {
  if (!IsValid())
    return;

  if (!callback_.is_null()) {
    callback_.Run(success);
    callback_.Reset();
  }
  // On Views, we can end up coming through this code path twice :(.
  // See crbug.com/63732.
  AppModalDialog::Invalidate();
}

}  // namespace xwalk
