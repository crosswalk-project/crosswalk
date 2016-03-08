// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_ui_delegate_desktop.h"

#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

namespace xwalk {

RuntimeUIDelegateDesktop::RuntimeUIDelegateDesktop(
    Runtime* runtime, const NativeAppWindow::CreateParams& params)
  : DefaultRuntimeUIDelegate(runtime, params) {
}

RuntimeUIDelegateDesktop::~RuntimeUIDelegateDesktop() {
}

void RuntimeUIDelegateDesktop::OnBackPressed() {
  runtime_->Back();
}

void RuntimeUIDelegateDesktop::OnForwardPressed() {
  runtime_->Forward();
}

void RuntimeUIDelegateDesktop::OnReloadPressed() {
  runtime_->Reload();
}

void RuntimeUIDelegateDesktop::OnStopPressed() {
  runtime_->Stop();
}

void RuntimeUIDelegateDesktop::OnApplicationExitRequested() {
  runtime_->RequestApplicationExit();
}

void RuntimeUIDelegateDesktop::SetLoadProgress(double progress) {
  if (NativeAppWindowDesktop* window = ToNativeAppWindowDesktop(GetAppWindow()))
    window->SetLoadProgress(progress);
}

void RuntimeUIDelegateDesktop::SetAddressURL(const GURL& url) {
  if (NativeAppWindowDesktop* window = ToNativeAppWindowDesktop(GetAppWindow()))
    window->SetAddressURL(url.spec());
}

bool RuntimeUIDelegateDesktop::AddDownloadItem(
    content::DownloadItem* download_item,
    const content::DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) {
  if (NativeAppWindowDesktop* window =
      ToNativeAppWindowDesktop(GetAppWindow())) {
    window->AddDownloadItem(download_item, callback, suggested_path);
    return true;
  }
  return false;
}

}  // namespace xwalk
