// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_MAC_H_
#define XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_MAC_H_

#import <Cocoa/Cocoa.h>

#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/rect.h"

namespace xwalk {

class NativeAppWindowMac : public NativeAppWindow {
 public:
  explicit NativeAppWindowMac(const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowMac();

  // NativeAppWindow implementation.
  gfx::NativeWindow GetNativeWindow() const override;
  void UpdateIcon(const gfx::Image& icon) override;
  void UpdateTitle(const base::string16& title) override;
  gfx::Rect GetRestoredBounds() const override;
  gfx::Rect GetBounds() const override;
  void SetBounds(const gfx::Rect& bounds) override;
  void Focus() override;
  void Show() override;
  void Hide() override;
  void Maximize() override;
  void Minimize() override;
  void SetFullscreen(bool fullscreen) override;
  void Restore() override;
  void FlashFrame(bool flash) override;
  void Close() override;
  bool IsActive() const override;
  bool IsMaximized() const override;
  bool IsMinimized() const override;
  bool IsFullscreen() const override;

 protected:
  content::WebContents* web_contents_;

  NSWindow* window_;
  base::string16 title_;

  bool is_fullscreen_;
  gfx::Size minimum_size_;
  gfx::Size maximum_size_;
  bool resizable_;
  NSInteger attention_request_id_;  // identifier from requestUserAttention

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowMac);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_MAC_H_
