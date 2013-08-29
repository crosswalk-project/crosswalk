// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_DESKTOP_ROOT_WINDOW_HOST_XWALK_H_
#define XWALK_RUNTIME_BROWSER_UI_DESKTOP_ROOT_WINDOW_HOST_XWALK_H_

#include <X11/Xlib.h>
#include <set>

// Get rid of a macro from Xlib.h that conflicts with Aura's RootWindow class.
#undef RootWindow

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "ui/aura/client/cursor_client.h"
#include "ui/aura/root_window_host.h"
#include "ui/aura/root_window_observer.h"
#include "ui/base/cursor/cursor_loader_x11.h"
#include "ui/base/x/x11_atom_cache.h"
#include "ui/gfx/rect.h"
#include "ui/views/views_export.h"
#include "ui/views/widget/desktop_aura/desktop_root_window_host.h"

namespace aura {
class RootWindow;
namespace client {
class FocusClient;
class ScreenPositionClient;
}
}

namespace views {
class DesktopActivationClient;
class DesktopCaptureClient;
class DesktopDispatcherClient;
class X11DesktopWindowMoveClient;
class X11WindowEventFilter;

namespace corewm {
class CursorManager;
}

class VIEWS_EXPORT DesktopRootWindowHostXWalk : public DesktopRootWindowHost,
    public aura::RootWindowHost,
    public aura::RootWindowObserver,
    public base::MessageLoop::Dispatcher {
 public:
  static DesktopRootWindowHost* Create(
      internal::NativeWidgetDelegate* native_widget_delegate,
      DesktopNativeWidgetAura* desktop_native_widget_aura,
      const gfx::Rect& initial_bounds);

  DesktopRootWindowHostXWalk(
      internal::NativeWidgetDelegate* native_widget_delegate,
      DesktopNativeWidgetAura* desktop_native_widget_aura,
      const gfx::Rect& initial_bounds);
  virtual ~DesktopRootWindowHostXWalk();

  // A way of converting an X11 |xid| host window into a |content_window_|.
  static aura::Window* GetContentWindowForXID(XID xid);

  // A way of converting an X11 |xid| host window into this object.
  static DesktopRootWindowHostXWalk* GetHostForXID(XID xid);

  // Called by X11DesktopHandler to notify us that the native windowing system
  // has changed our activation.
  void HandleNativeWidgetActivationChanged(bool active);

 private:
  // Initializes our X11 surface to draw on. This method performs all
  // initialization related to talking to the X11 server.
  void InitX11Window(const Widget::InitParams& params);

  // Creates an aura::RootWindow to contain the |content_window|, along with
  // all aura client objects that direct behavior.
  aura::RootWindow* InitRootWindow(const Widget::InitParams& params);

  // Returns true if there's an X window manager present... in most cases.  Some
  // window managers (notably, ion3) don't implement enough of ICCCM for us to
  // detect that they're there.
  bool IsWindowManagerPresent();

  // Sends a message to the x11 window manager, enabling or disabling the
  // states |state1| and |state2|.
  void SetWMSpecState(bool enabled, ::Atom state1, ::Atom state2);

  // Checks if the window manager has set a specific state.
  bool HasWMSpecProperty(const char* property) const;

  // Called when another DRWHL takes capture, or when capture is released
  // entirely.
  void OnCaptureReleased();

  // Dispatches a mouse event, taking mouse capture into account. If a
  // different host has capture, we translate the event to its coordinate space
  // and dispatch it to that host instead.
  void DispatchMouseEvent(ui::MouseEvent* event);

  // Overridden from DesktopRootWindowHost:
  virtual aura::RootWindow* Init(aura::Window* content_window,
                                 const Widget::InitParams& params) OVERRIDE;
  virtual void InitFocus(aura::Window* window) OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual void CloseNow() OVERRIDE;
  virtual aura::RootWindowHost* AsRootWindowHost() OVERRIDE;
  virtual void ShowWindowWithState(ui::WindowShowState show_state) OVERRIDE;
  virtual void ShowMaximizedWithBounds(
      const gfx::Rect& restored_bounds) OVERRIDE;
  virtual bool IsVisible() const OVERRIDE;
  virtual void SetSize(const gfx::Size& size) OVERRIDE;
  virtual void CenterWindow(const gfx::Size& size) OVERRIDE;
  virtual void GetWindowPlacement(
      gfx::Rect* bounds,
      ui::WindowShowState* show_state) const OVERRIDE;
  virtual gfx::Rect GetWindowBoundsInScreen() const OVERRIDE;
  virtual gfx::Rect GetClientAreaBoundsInScreen() const OVERRIDE;
  virtual gfx::Rect GetRestoredBounds() const OVERRIDE;
  virtual gfx::Rect GetWorkAreaBoundsInScreen() const OVERRIDE;
  virtual void SetShape(gfx::NativeRegion native_region) OVERRIDE;
  virtual void Activate() OVERRIDE;
  virtual void Deactivate() OVERRIDE;
  virtual bool IsActive() const OVERRIDE;
  virtual void Maximize() OVERRIDE;
  virtual void Minimize() OVERRIDE;
  virtual void Restore() OVERRIDE;
  virtual bool IsMaximized() const OVERRIDE;
  virtual bool IsMinimized() const OVERRIDE;
  virtual bool HasCapture() const OVERRIDE;
  virtual void SetAlwaysOnTop(bool always_on_top) OVERRIDE;
  virtual void SetWindowTitle(const string16& title) OVERRIDE;
  virtual void ClearNativeFocus() OVERRIDE;
  virtual Widget::MoveLoopResult RunMoveLoop(
      const gfx::Vector2d& drag_offset,
      Widget::MoveLoopSource source) OVERRIDE;
  virtual void EndMoveLoop() OVERRIDE;
  virtual void SetVisibilityChangedAnimationsEnabled(bool value) OVERRIDE;
  virtual bool ShouldUseNativeFrame() OVERRIDE;
  virtual void FrameTypeChanged() OVERRIDE;
  virtual NonClientFrameView* CreateNonClientFrameView() OVERRIDE;
  virtual void SetFullscreen(bool fullscreen) OVERRIDE;
  virtual bool IsFullscreen() const OVERRIDE;
  virtual void SetOpacity(unsigned char opacity) OVERRIDE;
  virtual void SetWindowIcons(const gfx::ImageSkia& window_icon,
                              const gfx::ImageSkia& app_icon) OVERRIDE;
  virtual void InitModalType(ui::ModalType modal_type) OVERRIDE;
  virtual void FlashFrame(bool flash_frame) OVERRIDE;
  virtual void OnNativeWidgetFocus() OVERRIDE;
  virtual void OnNativeWidgetBlur() OVERRIDE;
  virtual void SetInactiveRenderingDisabled(bool disable_inactive) OVERRIDE;

  // Overridden from aura::RootWindowHost:
  virtual void SetDelegate(aura::RootWindowHostDelegate* delegate) OVERRIDE;
  virtual aura::RootWindow* GetRootWindow() OVERRIDE;
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  virtual void Show() OVERRIDE;
  virtual void Hide() OVERRIDE;
  virtual void ToggleFullScreen() OVERRIDE;
  virtual gfx::Rect GetBounds() const OVERRIDE;
  virtual void SetBounds(const gfx::Rect& bounds) OVERRIDE;
  virtual gfx::Insets GetInsets() const OVERRIDE;
  virtual void SetInsets(const gfx::Insets& insets) OVERRIDE;
  virtual gfx::Point GetLocationOnNativeScreen() const OVERRIDE;
  virtual void SetCapture() OVERRIDE;
  virtual void ReleaseCapture() OVERRIDE;
  virtual void SetCursor(gfx::NativeCursor cursor) OVERRIDE;
  virtual bool QueryMouseLocation(gfx::Point* location_return) OVERRIDE;
  virtual bool ConfineCursorToRootWindow() OVERRIDE;
  virtual void UnConfineCursor() OVERRIDE;
  virtual void OnCursorVisibilityChanged(bool show) OVERRIDE;
  virtual void MoveCursorTo(const gfx::Point& location) OVERRIDE;
  virtual void SetFocusWhenShown(bool focus_when_shown) OVERRIDE;
  virtual bool CopyAreaToSkCanvas(const gfx::Rect& source_bounds,
                                  const gfx::Point& dest_offset,
                                  SkCanvas* canvas) OVERRIDE;
  virtual void PostNativeEvent(const base::NativeEvent& native_event) OVERRIDE;
  virtual void OnDeviceScaleFactorChanged(float device_scale_factor) OVERRIDE;
  virtual void PrepareForShutdown() OVERRIDE;

  // Overridden from Dispatcher:
  virtual bool Dispatch(const base::NativeEvent& event) OVERRIDE;

  // Overriden from aura::RootWindowObserver:
  virtual void OnRootWindowHostCloseRequested(
      const aura::RootWindow* root) OVERRIDE;

  base::WeakPtrFactory<DesktopRootWindowHostXWalk> close_widget_factory_;

  // X11 things
  // The display and the native X window hosting the root window.
  Display* xdisplay_;
  ::Window xwindow_;

  // The native root window.
  ::Window x_root_window_;

  ui::X11AtomCache atom_cache_;

  // Is the window mapped to the screen?
  bool window_mapped_;

  // The bounds of |xwindow_|.
  gfx::Rect bounds_;

  // True if the window should be focused when the window is shown.
  bool focus_when_shown_;

  // The window manager state bits.
  std::set< ::Atom> window_properties_;

  // We are owned by the RootWindow, but we have to have a back pointer to it.
  aura::RootWindow* root_window_;

  // aura:: objects that we own.
  scoped_ptr<DesktopCaptureClient> capture_client_;
  scoped_ptr<aura::client::FocusClient> focus_client_;
  scoped_ptr<DesktopActivationClient> activation_client_;
  scoped_ptr<views::corewm::CursorManager> cursor_client_;
  scoped_ptr<DesktopDispatcherClient> dispatcher_client_;
  scoped_ptr<aura::client::ScreenPositionClient> position_client_;

  // Current Aura cursor.
  gfx::NativeCursor current_cursor_;

  scoped_ptr<X11WindowEventFilter> x11_window_event_filter_;
  scoped_ptr<X11DesktopWindowMoveClient> x11_window_move_client_;

  // TODO(beng): Consider providing an interface to DesktopNativeWidgetAura
  //             instead of providing this route back to Widget.
  internal::NativeWidgetDelegate* native_widget_delegate_;

  DesktopNativeWidgetAura* desktop_native_widget_aura_;

  aura::RootWindowHostDelegate* root_window_host_delegate_;
  aura::Window* content_window_;

  // The current root window host that has capture. While X11 has something
  // like Windows SetCapture()/ReleaseCapture(), it is entirely implicit and
  // there are no notifications when this changes. We need to track this so we
  // can notify widgets when they have lost capture, which controls a bunch of
  // things in views like hiding menus.
  static DesktopRootWindowHostXWalk* g_current_capture;

  DISALLOW_COPY_AND_ASSIGN(DesktopRootWindowHostXWalk);
};

}  // namespace views

#endif  // XWALK_RUNTIME_BROWSER_UI_DESKTOP_ROOT_WINDOW_HOST_XWALK_H_
