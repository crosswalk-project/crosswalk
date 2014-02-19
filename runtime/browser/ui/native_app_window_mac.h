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
  virtual gfx::NativeWindow GetNativeWindow() const OVERRIDE;
  virtual void UpdateIcon(const gfx::Image& icon) OVERRIDE;
  virtual void UpdateTitle(const base::string16& title) OVERRIDE;
  virtual gfx::Rect GetRestoredBounds() const OVERRIDE;
  virtual gfx::Rect GetBounds() const OVERRIDE;
  virtual void SetBounds(const gfx::Rect& bounds) OVERRIDE;
  virtual void Focus() OVERRIDE;
  virtual void Show() OVERRIDE;
  virtual void Hide() OVERRIDE;
  virtual void Maximize() OVERRIDE;
  virtual void Minimize() OVERRIDE;
  virtual void SetFullscreen(bool fullscreen) OVERRIDE;
  virtual void Restore() OVERRIDE;
  virtual void FlashFrame(bool flash) OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual bool IsActive() const OVERRIDE;
  virtual bool IsMaximized() const OVERRIDE;
  virtual bool IsMinimized() const OVERRIDE;
  virtual bool IsFullscreen() const OVERRIDE;

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
