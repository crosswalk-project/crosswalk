// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/ui/app_modal_dialogs/app_modal_dialog_queue.h"

#include "base/memory/singleton.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/app_modal_dialog.h"

// static
AppModalDialogQueue* AppModalDialogQueue::GetInstance() {
  return Singleton<AppModalDialogQueue>::get();
}

void AppModalDialogQueue::AddDialog(AppModalDialog* dialog) {
  if (!active_dialog_) {
    ShowModalDialog(dialog);
    return;
  }
  app_modal_dialog_queue_.push_back(dialog);
}

void AppModalDialogQueue::ShowNextDialog() {
  AppModalDialog* dialog = GetNextDialog();
  if (dialog)
    ShowModalDialog(dialog);
  else
    active_dialog_ = NULL;
}

void AppModalDialogQueue::ActivateModalDialog() {
  if (showing_modal_dialog_) {
    // As part of showing a modal dialog we may end up back in this method
    // (showing a dialog activates the WebContents, which can trigger a call
    // to ActivateModalDialog). We ignore such a request as after the call to
    // activate the tab contents the dialog is shown.
    return;
  }
  if (active_dialog_)
    active_dialog_->ActivateModalDialog();
}

bool AppModalDialogQueue::HasActiveDialog() const {
  return active_dialog_ != NULL;
}

AppModalDialogQueue::AppModalDialogQueue()
    : active_dialog_(NULL),
      showing_modal_dialog_(false) {
}

AppModalDialogQueue::~AppModalDialogQueue() {
}

void AppModalDialogQueue::ShowModalDialog(AppModalDialog* dialog) {
  // Be sure and set the active_dialog_ field first, otherwise if
  // ShowModalDialog triggers a call back to the queue they'll get the old
  // dialog. Also, if the dialog calls |ShowNextDialog()| before returning, that
  // would write NULL into |active_dialog_| and this function would then undo
  // that.
  active_dialog_ = dialog;
  showing_modal_dialog_ = true;
  dialog->ShowModalDialog();
  showing_modal_dialog_ = false;
}

AppModalDialog* AppModalDialogQueue::GetNextDialog() {
  while (!app_modal_dialog_queue_.empty()) {
    AppModalDialog* dialog = app_modal_dialog_queue_.front();
    app_modal_dialog_queue_.pop_front();
    if (dialog->IsValid())
      return dialog;
    delete dialog;
  }
  return NULL;
}
