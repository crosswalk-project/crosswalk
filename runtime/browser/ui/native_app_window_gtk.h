// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_GTK_H_
#define XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_GTK_H_

#include <gtk/gtk.h>
#include <string>

#include "base/memory/scoped_ptr.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "third_party/skia/include/core/SkRegion.h"
#include "ui/base/gtk/gtk_signal.h"
#include "ui/base/x/active_window_watcher_x_observer.h"

namespace xwalk {

class NativeAppWindowGtk : public NativeAppWindow,
    public ui::ActiveWindowWatcherXObserver {
 public:
  explicit NativeAppWindowGtk(const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowGtk();

  // NativeAppWindow implementation.
  virtual gfx::NativeWindow GetNativeWindow() const OVERRIDE { return window_; }
  virtual void UpdateIcon(const gfx::Image& icon) OVERRIDE;
  virtual void UpdateTitle(const string16& title) OVERRIDE;
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

  // ActiveWindowWatcherXObserver implementation.
  virtual void ActiveWindowChanged(GdkWindow* active_window) OVERRIDE;

 private:
  // A set of helper functions.
  // TODO(hmin): Is possible to extract them into a util file?
  static void SetMinimumSize(GtkWindow* window, const gfx::Size& size);
  static void SetMaximumSize(GtkWindow* window, const gfx::Size& size);
  static void SetResizable(GtkWindow* window, bool resizable);
  static void SetWindowSize(GtkWindow* window, const gfx::Size& size);
  void SetWebKitColorStyle(GtkWindow* window);

  CHROMEGTK_CALLBACK_0(NativeAppWindowGtk, gboolean, OnWindowDestroyed);
  CHROMEGTK_CALLBACK_1(NativeAppWindowGtk, gboolean, OnWindowState,
                       GdkEventWindowState*);
  CHROMEGTK_CALLBACK_1(NativeAppWindowGtk, gboolean, OnWindowDeleteEvent,
                       GdkEvent*);

  NativeAppWindowDelegate* delegate_;
  content::WebContents* web_contents_;

  gfx::Size minimum_size_;
  gfx::Size maximum_size_;
  bool is_fullscreen_;
  bool resizable_;

  GtkWindow* window_;
  GtkWidget* vbox_;
  GdkWindowState state_;
  bool is_active_;

  // If true, don't call gdk_window_raise() when we get a click in the title
  // bar or window border.  This is to work around a compiz bug.
  bool suppress_window_raise_;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowGtk);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_GTK_H_
