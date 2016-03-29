// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_DESKTOP_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_DESKTOP_H_

#include "xwalk/runtime/browser/runtime_ui_delegate.h"
#include "xwalk/runtime/browser/ui/native_app_window_desktop.h"

namespace content {
class WebContents;
}

namespace xwalk {
class Runtime;

// Worked as the window delegate and views controller on desktop platforms.
class RuntimeUIDelegateDesktop: public DefaultRuntimeUIDelegate {
 public:
  RuntimeUIDelegateDesktop(Runtime* runtime,
                         const NativeAppWindow::CreateParams& params);
  ~RuntimeUIDelegateDesktop() override;

 private:
  // DefaultRuntimeUIDelegate.
  void SetLoadProgress(double progress) override;
  void SetAddressURL(const GURL& url) override;
  void OnBackPressed() override;
  void OnForwardPressed() override;
  void OnReloadPressed() override;
  void OnStopPressed() override;
  void OnApplicationExitRequested() override;
  bool AddDownloadItem(content::DownloadItem* download_item,
      const content::DownloadTargetCallback& callback,
      const base::FilePath& suggested_path) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(RuntimeUIDelegateDesktop);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_DESKTOP_H_
