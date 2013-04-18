// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_UI_NATIVE_APP_WINDOW_H_
#define CAMEO_SRC_BROWSER_UI_NATIVE_APP_WINDOW_H_

#include "base/compiler_specific.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"

namespace gfx {
class Rect;
}

namespace content {
struct NativeWebKeyboardEvent;
class WebContents;
}

namespace cameo {

class Shell;

// Base window class for native application.
class NativeAppWindow {
 public:
  enum State {
    STATE_NORMAL = 0, // Normal window.
    STATE_FULLSCREEN, // Fullscreen mode.
    STATE_MAXIMIZED,  // Maximized mode.
    STATE_MINIMIZED   // Minimized mode.
  };

  struct CreateParams {
    CreateParams() : resizable(true), state(STATE_NORMAL) {}
    // The window size.
    gfx::Size size;
    // The minimum size.
    gfx::Size minimum_size;
    // The maximum size.
    gfx::Size maximum_size;
    // The window state.
    State state;
    // True if the window can be resized.
    bool resizable;
  };

  enum ButtonType {
    BUTTON_TYPE_BACK,
    BUTTON_TYPE_FORWARD,
    BUTTON_TYPE_REFRESH_OR_STOP,
    BUTTON_TYPE_DEVTOOLS
  };

  // Used by Shell to instantiate the platform-specific ShellWindow code.
  static NativeAppWindow* Create(Shell* shell,
                                 const CreateParams& params);

  virtual ~NativeAppWindow() {}

  virtual void SetFullscreen(bool fullscreen) = 0;

  // Called when the icon of the window changes.
  virtual void UpdateWindowIcon() = 0;

  // Called when the title of the window changes.
  virtual void SetTitle(const std::string& title) = 0;

  // Allows the window to handle unhandled keyboard messages coming back from
  // the renderer.
  virtual void HandleKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) = 0;

  // Returns true if the window is currently the active/focused window.
  virtual bool IsActive() const = 0;

  // Returns true if the window is maximized (aka zoomed).
  virtual bool IsMaximized() const = 0;

  // Returns true if the window is minimized.
  virtual bool IsMinimized() const = 0;

  // Returns true if the window is full screen.
  virtual bool IsFullscreen() const = 0;

  // Return a platform dependent identifier for this window.
  virtual gfx::NativeWindow GetNativeWindow() = 0;

  // Returns the nonmaximized bounds of the window (even if the window is
  // currently maximized or minimized) in terms of the screen coordinates.
  virtual gfx::Rect GetRestoredBounds() const = 0;

  // Retrieves the window's current bounds, including its window.
  // This will only differ from GetRestoredBounds() for maximized
  // and minimized windows.
  virtual gfx::Rect GetBounds() const = 0;

  // Focus the window.
  virtual void Focus() = 0;

  // Blur the widnow.
  virtual void Blur() = 0;

  // Shows the window, or activates it if it's already visible.
  virtual void Show() = 0;

  // Hides the window.
  virtual void Hide() = 0;

  // Show the window, but do not activate it. Does nothing if window
  // is already visible.
  virtual void ShowInactive() = 0;

  // Closes the window as soon as possible. The close action may be delayed
  // if an operation is in progress (e.g. a drag operation).
  virtual void Close() = 0;

  // Maximizes/minimizes/restores the window.
  virtual void Maximize() = 0;
  virtual void Minimize() = 0;
  virtual void Restore() = 0;

  // Sets the window's size and position to the specified values.
  virtual void SetBounds(const gfx::Rect& bounds) = 0;

  // Flashes the taskbar item associated with this window.
  // Set |flash| to true to initiate flashing, false to stop flashing.
  virtual void FlashFrame(bool flash) = 0;

  virtual void AddToolbar() = 0;
  virtual void SetToolbarButtonEnabled(ButtonType button, bool enabled) = 0;
  virtual void SetToolbarUrlEntry(const std::string& url) = 0;
  virtual void SetToolbarIsLoading(bool loading) = 0;

  virtual bool CanResize() const { return resizable_; }

  virtual bool CanMaximize() const {
    return resizable_ &&
      (maximum_size_.IsEmpty() || minimum_size_ != maximum_size_);
  }

  virtual bool has_frame() const { return has_frame_; }

  virtual gfx::Size GetMaximumSize() const { return maximum_size_; }

  virtual gfx::Size GetMinimumSize() const { return minimum_size_; }

  virtual gfx::Size GetSize() { return size_; }

  // TODO(hmin): Should we also need to expose setter APIs to change window
  // appearance? e.g. SetSize, SetAlwaysOnTop etc.

 protected:
  NativeAppWindow(cameo::Shell* shell, const CreateParams& params)
    : shell_(shell),
      resizable_(params.resizable),
      state_(params.state),
      size_(params.size),
      minimum_size_(params.minimum_size),
      maximum_size_(params.maximum_size),
      has_frame_(true) {
    // TODO(hmin): Need to validate the value of maximum size and minimum size.
  }

  // Weak reference to cameo shell.
  Shell* shell_;
  // The window size.
  gfx::Size size_;
  // The minimum size.
  gfx::Size minimum_size_;
  // The maximum size.
  gfx::Size maximum_size_;
  // The window state.
  State state_;
  // True if the window can be resized.
  bool resizable_;
  // True if the window has frame.
  bool has_frame_;
};

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_UI_NATIVE_APP_WINDOW_H_