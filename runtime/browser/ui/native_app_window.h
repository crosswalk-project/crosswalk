// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_H_
#define XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_H_

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/strings/string16.h"
#include "content/public/common/context_menu_params.h"
#include "third_party/WebKit/public/platform/WebDisplayMode.h"
#include "ui/base/ui_base_types.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/native_widget_types.h"

namespace content {
class WebContents;
}

namespace xwalk {

class NativeAppWindowDelegate {
 public:
  // Called when native app window is being destroyed.
  virtual void OnBackPressed() {}
  virtual void OnForwardPressed() {}
  virtual void OnReloadPressed() {}
  virtual void OnStopPressed() {}
  virtual void OnWindowDestroyed() {}
  virtual void OnApplicationExitRequested() {}

 protected:
  virtual ~NativeAppWindowDelegate() {}
};

// Base window class for native application.
class NativeAppWindow {
 public:
  struct CreateParams {
    CreateParams();
    CreateParams(const CreateParams& other);
    ~CreateParams();
    // Delegate for this window.
    NativeAppWindowDelegate* delegate;
    // WebContents which the content will be displayed in this window.
    content::WebContents* web_contents;
    // The initial window bounds, empty means default bound will be used.
    gfx::Rect bounds;
    // The minimum window size. The window can only be resized to smaller if
    // its width or height is greater than the value in |minimum_size|.
    gfx::Size minimum_size;
    // The maximum window size. The window can only be resized to bigger if
    // its width or height is lower than the value in |maximum_size|.
    gfx::Size maximum_size;
    // The window state, e.g. fullscreen, maximized and minimized.
    ui::WindowShowState state;
    // True if the window can be resized.
    bool resizable;
    // The parent view which this window belongs to. NULL if it is root window.
    gfx::NativeView parent;
    // The absolute path of splash screen.
    // Empty if splash screen is not to be shown.
    base::FilePath splash_screen_path;
    // The display mode. Used on Desktop platforms.
    blink::WebDisplayMode display_mode;
  };

  // Do one time initialization at application startup.
  static void Initialize();

  // Initialize the platform-specific native app window.
  static NativeAppWindow* Create(const CreateParams& params);

  // Return a platform dependent identifier for this window.
  virtual gfx::NativeWindow GetNativeWindow() const = 0;
  // Returns true if the window has no frame.
  // Called when the icon of the window changes.
  virtual void UpdateIcon(const gfx::Image& icon) = 0;
  // Called when the title of the window changes.
  virtual void UpdateTitle(const base::string16& title) = 0;
  // Returns the nonmaximized bounds of the window (even if the window is
  // currently maximized or minimized) in terms of the screen coordinates.
  virtual gfx::Rect GetRestoredBounds() const = 0;
  // Retrieves the window's current bounds, including its window.
  virtual gfx::Rect GetBounds() const = 0;
  // Sets the window's size and position to the specified values.
  virtual void SetBounds(const gfx::Rect& bounds) = 0;

  // Focus the native app window.
  virtual void Focus() = 0;
  // Shows the window, or activates it if it's already visible.
  virtual void Show() = 0;
  // Hides the window.
  virtual void Hide() = 0;
  // Maximizes the window.
  virtual void Maximize() = 0;
  // Minimized the window.
  virtual void Minimize() = 0;
  // Toggle the window fullscreen status.
  virtual void SetFullscreen(bool fullscreen) = 0;
  // Restore the window.
  virtual void Restore() = 0;
  // Flash the taskbar item associated with this window.
  // Set |flash| to true to initiate flashing, false to stop flashing.
  virtual void FlashFrame(bool flash) = 0;
  // Close the window as soon as possible. The close action may be delayed
  // if an operation is in progress (e.g. a drag operation).
  virtual void Close() = 0;
  // Close all the open windows and exit the application.
  virtual void ExitApplication() = 0;

  // Returns true if the window is currently the active/focused window.
  virtual bool IsActive() const = 0;
  virtual bool IsMaximized() const = 0;
  virtual bool IsMinimized() const = 0;
  virtual bool IsFullscreen() const = 0;

  virtual bool PlatformHandleContextMenu(
      const content::ContextMenuParams& params) = 0;

 protected:
  virtual ~NativeAppWindow() {}
};

}  // namespace xwalk
#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_H_
