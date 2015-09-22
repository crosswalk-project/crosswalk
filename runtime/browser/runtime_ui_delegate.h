// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_H_

#include "content/public/browser/download_item.h"
#include "content/public/browser/download_manager_delegate.h"
#include "url/gurl.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"

namespace xwalk {
class Runtime;

class RuntimeUIDelegate {
 public:
  static RuntimeUIDelegate* Create(Runtime* runtime,
                                   const NativeAppWindow::CreateParams& params =
                                       NativeAppWindow::CreateParams());

  virtual ~RuntimeUIDelegate() {}
  virtual NativeAppWindow* GetAppWindow() = 0;
  virtual void Show() = 0;
  virtual void UpdateTitle(const base::string16& text) = 0;
  virtual void UpdateIcon(const gfx::Image& image) = 0;
  virtual void SetFullscreen(bool enter_fullscreen) = 0;
  virtual void Close() = 0;
  virtual void DeleteDelegate() = 0;
  virtual void SetLoadProgress(double progress) = 0;
  virtual void SetAddressURL(const GURL& url) = 0;
  virtual bool AddDownloadItem(content::DownloadItem* download_item,
      const content::DownloadTargetCallback& callback,
      const base::FilePath& suggested_path) = 0;
  virtual blink::WebDisplayMode GetDisplayMode() const = 0;
  virtual bool HandleContextMenu(const content::ContextMenuParams& params) = 0;
};

// The default implementation displays WebContents in a separate window.
class DefaultRuntimeUIDelegate : public RuntimeUIDelegate,
                                 public NativeAppWindowDelegate {
 public:
  DefaultRuntimeUIDelegate(Runtime* runtime,
                           const NativeAppWindow::CreateParams& params);
  ~DefaultRuntimeUIDelegate() override;

 protected:
  // RuntimeUIDelegate
  NativeAppWindow* GetAppWindow() override;
  void Show() override;
  void UpdateTitle(const base::string16& text) override;
  void UpdateIcon(const gfx::Image& image) override;
  void SetFullscreen(bool enter_fullscreen) override;
  void Close() override;
  void DeleteDelegate() override;
  void SetLoadProgress(double progress) override {}
  void SetAddressURL(const GURL& url) override {}
  bool AddDownloadItem(content::DownloadItem* download_item,
      const content::DownloadTargetCallback& callback,
      const base::FilePath& suggested_path) override;
  blink::WebDisplayMode GetDisplayMode() const override;
  bool HandleContextMenu(const content::ContextMenuParams& params) override;

  Runtime* runtime_;

 private:
  // NativeAppWindowDelegate
  void OnWindowDestroyed() override;

  NativeAppWindow* window_;
  NativeAppWindow::CreateParams window_params_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_H_
