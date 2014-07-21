// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_AURA_H_
#define XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_AURA_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "ui/aura/window_tree_host.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"

namespace aura {
namespace client {
class DefaultCaptureClient;
class FocusClient;
class WindowTreeClient;
}
}

namespace gfx {
class Size;
}

namespace ui {
class EventHandler;
}

namespace wm {
class CursorManager;
}

namespace xwalk {

class TopViewLayout;

class NativeAppWindowAura : public NativeAppWindow {
 public:
  explicit NativeAppWindowAura(const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowAura();

  // NativeAppWindow implementation.
  virtual gfx::NativeWindow GetNativeWindow() const override;
  virtual void UpdateIcon(const gfx::Image& icon) override;
  virtual void UpdateTitle(const base::string16& title) override;
  virtual gfx::Rect GetRestoredBounds() const override;
  virtual gfx::Rect GetBounds() const override;
  virtual void SetBounds(const gfx::Rect& bounds) override;
  virtual void Focus() override;
  virtual void Show() override;
  virtual void Hide() override;
  virtual void Maximize() override;
  virtual void Minimize() override;
  virtual void SetFullscreen(bool fullscreen) override;
  virtual void Restore() override;
  virtual void FlashFrame(bool flash) override;
  virtual void Close() override;
  virtual bool IsActive() const override;
  virtual bool IsMaximized() const override;
  virtual bool IsMinimized() const override;
  virtual bool IsFullscreen() const override;

 private:
  scoped_ptr<aura::WindowTreeHost> host_;
  scoped_ptr<aura::client::FocusClient> focus_client_;
  scoped_ptr<wm::CursorManager> cursor_manager_;
  scoped_ptr<aura::client::DefaultCaptureClient> capture_client_;
  scoped_ptr<aura::client::WindowTreeClient> window_tree_client_;
  scoped_ptr<ui::EventHandler> ime_filter_;

  content::WebContents* web_contents_;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowAura);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_AURA_H_
