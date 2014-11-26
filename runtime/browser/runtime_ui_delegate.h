// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_H_

#include "base/memory/scoped_ptr.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"

namespace xwalk {
class Runtime;

class RuntimeUIDelegate {
 public:
  virtual ~RuntimeUIDelegate() {}
  virtual void Show() = 0;
  virtual void UpdateTitle(const base::string16& text) = 0;
  virtual void UpdateIcon(const gfx::Image& image) = 0;
  virtual void SetFullscreen(bool enter_fullscreen) = 0;
  virtual void Close() = 0;
  virtual void DeleteDelegate() = 0;
};

// The default implementation displays WebContents in a separate window.
class DefaultRuntimeUIDelegate : public RuntimeUIDelegate,
                                 public NativeAppWindowDelegate {
 public:
  static RuntimeUIDelegate* Create(
      Runtime* runtime,
      const NativeAppWindow::CreateParams& params =
          NativeAppWindow::CreateParams());
  virtual ~DefaultRuntimeUIDelegate();

  NativeAppWindow* window() { return window_; }

 private:
  DefaultRuntimeUIDelegate(Runtime* runtime,
                           const NativeAppWindow::CreateParams& params);
  // RuntimeUIDelegate
  virtual void Show() override;
  virtual void UpdateTitle(const base::string16& text) override;
  virtual void UpdateIcon(const gfx::Image& image) override;
  virtual void SetFullscreen(bool enter_fullscreen) override;
  virtual void Close() override;
  virtual void DeleteDelegate() override;
  // NativeAppWindowDelegate
  virtual void OnWindowDestroyed() override;

 private:
  Runtime* runtime_;
  NativeAppWindow::CreateParams window_params_;
  NativeAppWindow* window_;
};


}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_UI_DELEGATE_H_
