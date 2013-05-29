// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/ui/app_modal_dialogs/app_modal_dialog.h"

#include "base/logging.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/app_modal_dialog_queue.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/native_app_modal_dialog.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"

using content::WebContents;

AppModalDialog::AppModalDialog(WebContents* web_contents, const string16& title)
    : valid_(true),
      native_dialog_(NULL),
      title_(title),
      web_contents_(web_contents),
      completed_(false) {
}

AppModalDialog::~AppModalDialog() {
  CompleteDialog();
}

void AppModalDialog::ShowModalDialog() {
  web_contents_->GetDelegate()->ActivateContents(web_contents_);
  CreateAndShowDialog();
}

void AppModalDialog::CreateAndShowDialog() {
  native_dialog_ = CreateNativeDialog();
  native_dialog_->ShowAppModalDialog();
}

bool AppModalDialog::IsValid() {
  return valid_;
}

void AppModalDialog::Invalidate() {
  valid_ = false;
}

bool AppModalDialog::IsJavaScriptModalDialog() {
  return false;
}

void AppModalDialog::ActivateModalDialog() {
  DCHECK(native_dialog_);
  native_dialog_->ActivateAppModalDialog();
}

void AppModalDialog::CloseModalDialog() {
  DCHECK(native_dialog_);
  native_dialog_->CloseAppModalDialog();
}

void AppModalDialog::CompleteDialog() {
  if (!completed_) {
    completed_ = true;
    AppModalDialogQueue::GetInstance()->ShowNextDialog();
  }
}
