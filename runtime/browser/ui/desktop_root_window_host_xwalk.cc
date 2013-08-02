// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop_root_window_host_xwalk.h"

#include <X11/extensions/XInput2.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <algorithm>
#include <vector>

#include "base/message_pump_aurax11.h"
#include "base/stringprintf.h"
#include "base/utf_string_conversions.h"
#include "ui/aura/client/aura_constants.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/client/user_action_client.h"
#include "ui/aura/focus_manager.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window_property.h"
#include "ui/base/dragdrop/os_exchange_data_provider_aurax11.h"
#include "ui/base/events/event_utils.h"
#include "ui/base/touch/touch_factory_x11.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/insets.h"
#include "ui/linux_ui/linux_ui.h"
#include "ui/native_theme/native_theme.h"
#include "ui/views/corewm/compound_event_filter.h"
#include "ui/views/corewm/corewm_switches.h"
#include "ui/views/corewm/cursor_manager.h"
#include "ui/views/corewm/focus_controller.h"
#include "ui/views/ime/input_method.h"
#include "ui/views/widget/desktop_aura/desktop_activation_client.h"
#include "ui/views/widget/desktop_aura/desktop_capture_client.h"
#include "ui/views/widget/desktop_aura/desktop_cursor_loader_updater_aurax11.h"
#include "ui/views/widget/desktop_aura/desktop_dispatcher_client.h"
#include "ui/views/widget/desktop_aura/desktop_drag_drop_client_aurax11.h"
#include "ui/views/widget/desktop_aura/desktop_focus_rules.h"
#include "ui/views/widget/desktop_aura/desktop_layout_manager.h"
#include "ui/views/widget/desktop_aura/desktop_native_cursor_manager.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/desktop_aura/desktop_screen_position_client.h"
#include "ui/views/widget/desktop_aura/x11_desktop_handler.h"
#include "ui/views/widget/desktop_aura/x11_desktop_window_move_client.h"
#include "ui/views/widget/desktop_aura/x11_window_event_filter.h"

namespace views {

DesktopRootWindowHostXWalk* DesktopRootWindowHostXWalk::g_current_capture =
    NULL;

DEFINE_WINDOW_PROPERTY_KEY(
    aura::Window*, kViewsWindowForRootWindow, NULL);

DEFINE_WINDOW_PROPERTY_KEY(
    DesktopRootWindowHostXWalk*, kHostForRootWindow, NULL);

namespace {

// Standard Linux mouse buttons for going back and forward.
const int kBackMouseButton = 8;
const int kForwardMouseButton = 9;

// Constants that are part of EWMH.
const int k_NET_WM_STATE_ADD = 1;
const int k_NET_WM_STATE_REMOVE = 0;

const char* kAtomsToCache[] = {
  "WM_DELETE_WINDOW",
  "WM_PROTOCOLS",
  "WM_S0",
  "_NET_WM_PID",
  "_NET_WM_PING",
  "_NET_WM_STATE",
  "_NET_WM_STATE_FULLSCREEN",
  "_NET_WM_STATE_HIDDEN",
  "_NET_WM_STATE_MAXIMIZED_HORZ",
  "_NET_WM_STATE_MAXIMIZED_VERT",
  "XdndActionAsk",
  "XdndActionCopy"
  "XdndActionLink",
  "XdndActionList",
  "XdndActionMove",
  "XdndActionPrivate",
  "XdndAware",
  "XdndDrop",
  "XdndEnter",
  "XdndFinished",
  "XdndLeave",
  "XdndPosition",
  "XdndProxy",  // Proxy windows?
  "XdndSelection",
  "XdndStatus",
  "XdndTypeList",
  NULL
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostXWalk, public:

DesktopRootWindowHostXWalk::DesktopRootWindowHostXWalk(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura,
    const gfx::Rect& initial_bounds)
    : close_widget_factory_(this),
      xdisplay_(base::MessagePumpAuraX11::GetDefaultXDisplay()),
      xwindow_(0),
      x_root_window_(DefaultRootWindow(xdisplay_)),
      atom_cache_(xdisplay_, kAtomsToCache),
      window_mapped_(false),
      focus_when_shown_(false),
      current_cursor_(ui::kCursorNull),
      native_widget_delegate_(native_widget_delegate),
      desktop_native_widget_aura_(desktop_native_widget_aura),
      drop_handler_(NULL) {
}

DesktopRootWindowHostXWalk::~DesktopRootWindowHostXWalk() {
  root_window_->ClearProperty(kHostForRootWindow);
  if (corewm::UseFocusControllerOnDesktop()) {
    aura::client::SetFocusClient(root_window_, NULL);
    aura::client::SetActivationClient(root_window_, NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostXWalk, private:

void DesktopRootWindowHostXWalk::InitX11Window(
    const Widget::InitParams& params) {
  unsigned long attribute_mask = CWBackPixmap; // NOLINT(*)
  XSetWindowAttributes swa;
  memset(&swa, 0, sizeof(swa));
  swa.background_pixmap = None;

  if (params.type == Widget::InitParams::TYPE_MENU) {
    swa.override_redirect = True;
    attribute_mask |= CWOverrideRedirect;
  }

  xwindow_ = XCreateWindow(
      xdisplay_, x_root_window_,
      params.bounds.x(), params.bounds.y(),
      params.bounds.width(), params.bounds.height(),
      0,               // border width
      CopyFromParent,  // depth
      InputOutput,
      CopyFromParent,  // visual
      attribute_mask,
      &swa);
  base::MessagePumpAuraX11::Current()->AddDispatcherForWindow(this, xwindow_);

  // TODO(erg): Maybe need to set a ViewProp here like in RWHL::RWHL().

  long event_mask = ButtonPressMask | ButtonReleaseMask | FocusChangeMask |  // NOLINT(*)
                    KeyPressMask | KeyReleaseMask |
                    EnterWindowMask | LeaveWindowMask |
                    ExposureMask | VisibilityChangeMask |
                    StructureNotifyMask | PropertyChangeMask |
                    PointerMotionMask;
  XSelectInput(xdisplay_, xwindow_, event_mask);
  XFlush(xdisplay_);

  if (base::MessagePumpForUI::HasXInput2())
    ui::TouchFactory::GetInstance()->SetupXI2ForXWindow(xwindow_);

  // TODO(erg): We currently only request window deletion events. We also
  // should listen for activation events and anything else that GTK+ listens
  // for, and do something useful.
  ::Atom protocols[2];
  protocols[0] = atom_cache_.GetAtom("WM_DELETE_WINDOW");
  protocols[1] = atom_cache_.GetAtom("_NET_WM_PING");
  XSetWMProtocols(xdisplay_, xwindow_, protocols, 2);

  // We need a WM_CLIENT_MACHINE and WM_LOCALE_NAME value so we integrate with
  // the desktop environment.
  XSetWMProperties(xdisplay_, xwindow_, NULL, NULL, NULL, 0, NULL, NULL, NULL);

  // Likewise, the X server needs to know this window's pid so it knows which
  // program to kill if the window hangs.
  pid_t pid = getpid();
  XChangeProperty(xdisplay_,
                  xwindow_,
                  atom_cache_.GetAtom("_NET_WM_PID"),
                  XA_CARDINAL,
                  32,
                  PropModeReplace,
                  reinterpret_cast<unsigned char*>(&pid), 1);
}

// static
aura::Window* DesktopRootWindowHostXWalk::GetContentWindowForXID(XID xid) {
  aura::RootWindow* root = aura::RootWindow::GetForAcceleratedWidget(xid);
  return root ? root->GetProperty(kViewsWindowForRootWindow) : NULL;
}

// static
DesktopRootWindowHostXWalk* DesktopRootWindowHostXWalk::GetHostForXID(XID xid) {
  aura::RootWindow* root = aura::RootWindow::GetForAcceleratedWidget(xid);
  return root ? root->GetProperty(kHostForRootWindow) : NULL;
}

void DesktopRootWindowHostXWalk::HandleNativeWidgetActivationChanged(
    bool active) {
  native_widget_delegate_->OnNativeWidgetActivationChanged(active);
  native_widget_delegate_->AsWidget()->GetRootView()->SchedulePaint();
}

// TODO(erg): This method should basically be everything I need form
// RootWindowHostX11::RootWindowHostX11().
aura::RootWindow* DesktopRootWindowHostXWalk::InitRootWindow(
    const Widget::InitParams& params) {
  bounds_ = params.bounds;

  aura::RootWindow::CreateParams rw_params(bounds_);
  rw_params.host = this;
  root_window_ = new aura::RootWindow(rw_params);
  root_window_->Init();
  root_window_->AddChild(content_window_);
  root_window_->SetLayoutManager(new DesktopLayoutManager(root_window_));
  root_window_->SetProperty(kViewsWindowForRootWindow, content_window_);
  root_window_->SetProperty(kHostForRootWindow, this);
  root_window_->SetProperty(aura::client::kShowStateKey, params.show_state);
  root_window_host_delegate_ = root_window_;

  // If we're given a parent, we need to mark ourselves as transient to another
  // window. Otherwise activation gets screwy.
  gfx::NativeView parent = params.parent;
  if (!params.child && params.parent)
    parent->AddTransientChild(content_window_);

  native_widget_delegate_->OnNativeWidgetCreated();

  capture_client_.reset(new views::DesktopCaptureClient(root_window_));
  aura::client::SetCaptureClient(root_window_, capture_client_.get());

  // Ensure that the X11DesktopHandler exists so that it dispatches activation
  // messages to us.
  X11DesktopHandler::get();

  if (corewm::UseFocusControllerOnDesktop()) {
    corewm::FocusController* focus_controller =
        new corewm::FocusController(new DesktopFocusRules);
    focus_client_.reset(focus_controller);
    aura::client::SetFocusClient(root_window_, focus_controller);
    aura::client::SetActivationClient(root_window_, focus_controller);
    root_window_->AddPreTargetHandler(focus_controller);
  } else {
    focus_client_.reset(new aura::FocusManager);
    aura::client::SetFocusClient(root_window_, focus_client_.get());
    activation_client_.reset(new DesktopActivationClient(root_window_));
  }

  dispatcher_client_.reset(new DesktopDispatcherClient);
  aura::client::SetDispatcherClient(root_window_,
                                    dispatcher_client_.get());

  cursor_client_.reset(
      new views::corewm::CursorManager(
          scoped_ptr<corewm::NativeCursorManager>(
              new views::DesktopNativeCursorManager(
                  root_window_,
                  scoped_ptr<DesktopCursorLoaderUpdater>(
                      new DesktopCursorLoaderUpdaterAuraX11)))));
  aura::client::SetCursorClient(root_window_,
                                cursor_client_.get());

  position_client_.reset(new DesktopScreenPositionClient);
  aura::client::SetScreenPositionClient(root_window_,
                                        position_client_.get());

  desktop_native_widget_aura_->InstallInputMethodEventFilter(root_window_);

  drag_drop_client_.reset(new DesktopDragDropClientAuraX11(
      this, root_window_, xdisplay_, xwindow_));
  aura::client::SetDragDropClient(root_window_, drag_drop_client_.get());

  // TODO(erg): Unify this code once the other consumer goes away.
  x11_window_event_filter_.reset(
      new X11WindowEventFilter(root_window_, activation_client_.get()));

  // We reuse the |remove_standard_frame| in to tell if the window border is
  // used. Note the |remove_standard_frame| is originally designed for
  // Windows, see comments in ui/views/widget.h.
  bool use_os_border = params.remove_standard_frame ? false : true;
  x11_window_event_filter_->SetUseHostWindowBorders(use_os_border);

  // DesktopRootWindowHost should handle the close event emitted by
  // window manager, e.g. press close button.
  //
  // For a frameless window (not use os border), the custom frame
  // view will draw the border and handle the close event by hit
  // test.
  if (use_os_border)
    root_window_->AddRootWindowObserver(this);

  desktop_native_widget_aura_->root_window_event_filter()->AddHandler(
      x11_window_event_filter_.get());

  x11_window_move_client_.reset(new X11DesktopWindowMoveClient);
  aura::client::SetWindowMoveClient(root_window_,
                                    x11_window_move_client_.get());

  focus_client_->FocusWindow(content_window_);
  return root_window_;
}

bool DesktopRootWindowHostXWalk::IsWindowManagerPresent() {
  // Per ICCCM 2.8, "Manager Selections", window managers should take ownership
  // of WM_Sn selections (where n is a screen number).
  return XGetSelectionOwner(
      xdisplay_, atom_cache_.GetAtom("WM_S0")) != None;
}

void DesktopRootWindowHostXWalk::SetWMSpecState(bool enabled,
                                                ::Atom state1,
                                                ::Atom state2) {
  XEvent xclient;
  memset(&xclient, 0, sizeof(xclient));
  xclient.type = ClientMessage;
  xclient.xclient.window = xwindow_;
  xclient.xclient.message_type = atom_cache_.GetAtom("_NET_WM_STATE");
  xclient.xclient.format = 32;
  xclient.xclient.data.l[0] =
      enabled ? k_NET_WM_STATE_ADD : k_NET_WM_STATE_REMOVE;
  xclient.xclient.data.l[1] = state1;
  xclient.xclient.data.l[2] = state2;
  xclient.xclient.data.l[3] = 1;
  xclient.xclient.data.l[4] = 0;

  XSendEvent(xdisplay_, x_root_window_, False,
             SubstructureRedirectMask | SubstructureNotifyMask,
             &xclient);
}

bool DesktopRootWindowHostXWalk::HasWMSpecProperty(const char* property) const {
  return window_properties_.find(atom_cache_.GetAtom(property)) !=
      window_properties_.end();
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostXWalk, DesktopRootWindowHost implementation:

aura::RootWindow* DesktopRootWindowHostXWalk::Init(
    aura::Window* content_window,
    const Widget::InitParams& params) {
  content_window_ = content_window;

  // TODO(erg): Check whether we *should* be building a RootWindowHost here, or
  // whether we should be proxying requests to another DRWHL.

  // In some situations, views tries to make a zero sized window, and that
  // makes us crash. Make sure we have valid sizes.
  Widget::InitParams sanitized_params = params;
  if (sanitized_params.bounds.width() == 0)
    sanitized_params.bounds.set_width(100);
  if (sanitized_params.bounds.height() == 0)
    sanitized_params.bounds.set_height(100);

  InitX11Window(sanitized_params);
  return InitRootWindow(sanitized_params);
}

void DesktopRootWindowHostXWalk::InitFocus(aura::Window* window) {
}

void DesktopRootWindowHostXWalk::Close() {
  // TODO(erg): Might need to do additional hiding tasks here.

  if (!close_widget_factory_.HasWeakPtrs()) {
    // And we delay the close so that if we are called from an ATL callback,
    // we don't destroy the window before the callback returned (as the caller
    // may delete ourselves on destroy and the ATL callback would still
    // dereference us when the callback returns).
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&DesktopRootWindowHostXWalk::CloseNow,
                   close_widget_factory_.GetWeakPtr()));
  }
}

void DesktopRootWindowHostXWalk::CloseNow() {
  if (xwindow_ == None)
    return;

  native_widget_delegate_->OnNativeWidgetDestroying();

  // Remove the event listeners we've installed. We need to remove these
  // because otherwise we get assert during ~RootWindow().
  desktop_native_widget_aura_->root_window_event_filter()->RemoveHandler(
      x11_window_event_filter_.get());

  // Actually free our native resources.
  base::MessagePumpAuraX11::Current()->RemoveDispatcherForWindow(xwindow_);
  XDestroyWindow(xdisplay_, xwindow_);
  xwindow_ = None;

  desktop_native_widget_aura_->OnHostClosed();
}

aura::RootWindowHost* DesktopRootWindowHostXWalk::AsRootWindowHost() {
  return this;
}

void DesktopRootWindowHostXWalk::ShowWindowWithState(
    ui::WindowShowState show_state) {
  if (show_state != ui::SHOW_STATE_DEFAULT &&
      show_state != ui::SHOW_STATE_NORMAL) {
    // Only forwarding to Show().
    NOTIMPLEMENTED();
  }

  Show();
}

void DesktopRootWindowHostXWalk::ShowMaximizedWithBounds(
    const gfx::Rect& restored_bounds) {
  // TODO(erg):
  NOTIMPLEMENTED();

  // TODO(erg): We shouldn't completely fall down here.
  Show();
}

bool DesktopRootWindowHostXWalk::IsVisible() const {
  return window_mapped_;
}

void DesktopRootWindowHostXWalk::SetSize(const gfx::Size& size) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::CenterWindow(const gfx::Size& size) {
  gfx::Rect parent_bounds = GetWorkAreaBoundsInScreen();

  // If |window_|'s transient parent bounds are big enough to contain |size|,
  // use them instead.
  if (content_window_->transient_parent()) {
    gfx::Rect transient_parent_rect =
        content_window_->transient_parent()->GetBoundsInScreen();
    if (transient_parent_rect.height() >= size.height() &&
        transient_parent_rect.width() >= size.width()) {
      parent_bounds = transient_parent_rect;
    }
  }

  gfx::Rect window_bounds(
      parent_bounds.x() + (parent_bounds.width() - size.width()) / 2,
      parent_bounds.y() + (parent_bounds.height() - size.height()) / 2,
      size.width(),
      size.height());
  // Don't size the window bigger than the parent, otherwise the user may not be
  // able to close or move it.
  window_bounds.AdjustToFit(parent_bounds);

  SetBounds(window_bounds);
}

void DesktopRootWindowHostXWalk::GetWindowPlacement(
    gfx::Rect* bounds,
    ui::WindowShowState* show_state) const {
  *bounds = bounds_;

  // TODO(erg): This needs a better implementation. For now, we're just pass
  // back the normal state until we keep track of this.
  *show_state = ui::SHOW_STATE_NORMAL;
}

gfx::Rect DesktopRootWindowHostXWalk::GetWindowBoundsInScreen() const {
  return bounds_;
}

gfx::Rect DesktopRootWindowHostXWalk::GetClientAreaBoundsInScreen() const {
  // TODO(erg): The NativeWidgetAura version returns |bounds_|, claiming its
  // needed for View::ConvertPointToScreen() to work
  // correctly. DesktopRootWindowHostWin::GetClientAreaBoundsInScreen() just
  // asks windows what it thinks the client rect is.
  //
  // Attempts to calculate the rect by asking the NonClientFrameView what it
  // thought its GetBoundsForClientView() were broke combobox drop down
  // placement.
  return bounds_;
}

gfx::Rect DesktopRootWindowHostXWalk::GetRestoredBounds() const {
  // TODO(erg):
  NOTIMPLEMENTED();
  return gfx::Rect();
}

gfx::Rect DesktopRootWindowHostXWalk::GetWorkAreaBoundsInScreen() const {
  std::vector<int> value;
  if (ui::GetIntArrayProperty(x_root_window_, "_NET_WORKAREA", &value) &&
      value.size() >= 4) {
    return gfx::Rect(value[0], value[1], value[2], value[3]);
  }

  // Fetch the geometry of the root window.
  Window root;
  int x, y;
  unsigned int width, height;
  unsigned int border_width, depth;
  if (!XGetGeometry(xdisplay_, x_root_window_, &root, &x, &y,
                    &width, &height, &border_width, &depth)) {
    NOTIMPLEMENTED();
    return gfx::Rect(0, 0, 10, 10);
  }

  return gfx::Rect(x, y, width, height);
}

void DesktopRootWindowHostXWalk::SetShape(gfx::NativeRegion native_region) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::Activate() {
  X11DesktopHandler::get()->ActivateWindow(xwindow_);
}

void DesktopRootWindowHostXWalk::Deactivate() {
  // Deactivating a window means activating nothing.
  X11DesktopHandler::get()->ActivateWindow(None);
}

bool DesktopRootWindowHostXWalk::IsActive() const {
  return X11DesktopHandler::get()->IsActiveWindow(xwindow_);
}

void DesktopRootWindowHostXWalk::Maximize() {
  SetWMSpecState(true,
                 atom_cache_.GetAtom("_NET_WM_STATE_MAXIMIZED_VERT"),
                 atom_cache_.GetAtom("_NET_WM_STATE_MAXIMIZED_HORZ"));
}

void DesktopRootWindowHostXWalk::Minimize() {
  XIconifyWindow(xdisplay_, xwindow_, 0);
}

void DesktopRootWindowHostXWalk::Restore() {
  SetWMSpecState(false,
                 atom_cache_.GetAtom("_NET_WM_STATE_MAXIMIZED_VERT"),
                 atom_cache_.GetAtom("_NET_WM_STATE_MAXIMIZED_HORZ"));
}

bool DesktopRootWindowHostXWalk::IsMaximized() const {
  return (HasWMSpecProperty("_NET_WM_STATE_MAXIMIZED_VERT") ||
          HasWMSpecProperty("_NET_WM_STATE_MAXIMIZED_HORZ"));
}

bool DesktopRootWindowHostXWalk::IsMinimized() const {
  return HasWMSpecProperty("_NET_WM_STATE_HIDDEN");
}

void DesktopRootWindowHostXWalk::OnCaptureReleased() {
  native_widget_delegate_->OnMouseCaptureLost();
  g_current_capture = NULL;
}

void DesktopRootWindowHostXWalk::DispatchMouseEvent(ui::MouseEvent* event) {
  if (!g_current_capture || g_current_capture == this) {
    root_window_host_delegate_->OnHostMouseEvent(event);
  } else {
    // Another DesktopRootWindowHostXWalk has installed itself as
    // capture. Translate the event's location and dispatch to the other.
    event->ConvertLocationToTarget(root_window_,
                                   g_current_capture->root_window_);
    g_current_capture->root_window_host_delegate_->OnHostMouseEvent(event);
  }
}

bool DesktopRootWindowHostXWalk::HasCapture() const {
  return g_current_capture == this;
}

void DesktopRootWindowHostXWalk::SetAlwaysOnTop(bool always_on_top) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::SetWindowTitle(const string16& title) {
  XmbSetWMProperties(xdisplay_, xwindow_, UTF16ToUTF8(title).c_str(), NULL,
      NULL, 0, NULL, NULL, NULL);
}

void DesktopRootWindowHostXWalk::ClearNativeFocus() {
  // This method is weird and misnamed. Instead of clearing the native focus,
  // it sets the focus to our |content_window_|, which will trigger a cascade
  // of focus changes into views.
  if (content_window_ && aura::client::GetFocusClient(content_window_) &&
      content_window_->Contains(
          aura::client::GetFocusClient(content_window_)->GetFocusedWindow())) {
    aura::client::GetFocusClient(content_window_)->FocusWindow(content_window_);
  }
}

Widget::MoveLoopResult DesktopRootWindowHostXWalk::RunMoveLoop(
    const gfx::Vector2d& drag_offset,
    Widget::MoveLoopSource source) {
  SetCapture();

  aura::client::WindowMoveSource window_move_source =
      source == Widget::MOVE_LOOP_SOURCE_MOUSE ?
      aura::client::WINDOW_MOVE_SOURCE_MOUSE :
      aura::client::WINDOW_MOVE_SOURCE_TOUCH;
  if (x11_window_move_client_->RunMoveLoop(content_window_, drag_offset,
      window_move_source) == aura::client::MOVE_SUCCESSFUL)
    return Widget::MOVE_LOOP_SUCCESSFUL;

  return Widget::MOVE_LOOP_CANCELED;
}

void DesktopRootWindowHostXWalk::EndMoveLoop() {
  x11_window_move_client_->EndMoveLoop();
}

void DesktopRootWindowHostXWalk::SetVisibilityChangedAnimationsEnabled(
    bool value) {
  // Much like the previous NativeWidgetGtk, we don't have anything to do here.
}

bool DesktopRootWindowHostXWalk::ShouldUseNativeFrame() {
  return false;
}

void DesktopRootWindowHostXWalk::FrameTypeChanged() {
}

NonClientFrameView* DesktopRootWindowHostXWalk::CreateNonClientFrameView() {
  return NULL;
}

void DesktopRootWindowHostXWalk::SetFullscreen(bool fullscreen) {
  SetWMSpecState(fullscreen,
                 atom_cache_.GetAtom("_NET_WM_STATE_FULLSCREEN"),
                 None);
}

bool DesktopRootWindowHostXWalk::IsFullscreen() const {
  return HasWMSpecProperty("_NET_WM_STATE_FULLSCREEN");
}

void DesktopRootWindowHostXWalk::SetOpacity(unsigned char opacity) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::SetWindowIcons(
    const gfx::ImageSkia& window_icon, const gfx::ImageSkia& app_icon) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::InitModalType(ui::ModalType modal_type) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::FlashFrame(bool flash_frame) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::OnNativeWidgetFocus() {
  native_widget_delegate_->AsWidget()->GetInputMethod()->OnFocus();
}

void DesktopRootWindowHostXWalk::OnNativeWidgetBlur() {
  if (xwindow_)
    native_widget_delegate_->AsWidget()->GetInputMethod()->OnBlur();
}

void DesktopRootWindowHostXWalk::SetInactiveRenderingDisabled(
    bool disable_inactive) {
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostXWalk, aura::RootWindowHost implementation:

void DesktopRootWindowHostXWalk::SetDelegate(
    aura::RootWindowHostDelegate* delegate) {
  root_window_host_delegate_ = delegate;
}

aura::RootWindow* DesktopRootWindowHostXWalk::GetRootWindow() {
  return root_window_;
}

gfx::AcceleratedWidget DesktopRootWindowHostXWalk::GetAcceleratedWidget() {
  return xwindow_;
}

void DesktopRootWindowHostXWalk::Show() {
  if (!window_mapped_) {
    // Before we map the window, set size hints. Otherwise, some window managers
    // will ignore toplevel XMoveWindow commands.
    XSizeHints size_hints;
    size_hints.flags = PPosition;
    size_hints.x = bounds_.x();
    size_hints.y = bounds_.y();
    XSetWMNormalHints(xdisplay_, xwindow_, &size_hints);

    XMapWindow(xdisplay_, xwindow_);

    // We now block until our window is mapped. Some X11 APIs will crash and
    // burn if passed |xwindow_| before the window is mapped, and XMapWindow is
    // asynchronous.
    base::MessagePumpAuraX11::Current()->BlockUntilWindowMapped(xwindow_);
    window_mapped_ = true;

    // In some window managers, the window state change only takes effect after
    // the window gets mapped.
    ui::WindowShowState show_state =
        root_window_->GetProperty(aura::client::kShowStateKey);
    if (show_state == ui::SHOW_STATE_FULLSCREEN)
      SetFullscreen(true);
  }
}

void DesktopRootWindowHostXWalk::Hide() {
  if (window_mapped_) {
    XWithdrawWindow(xdisplay_, xwindow_, 0);
    window_mapped_ = false;
  }
}

void DesktopRootWindowHostXWalk::ToggleFullScreen() {
  NOTIMPLEMENTED();
}

gfx::Rect DesktopRootWindowHostXWalk::GetBounds() const {
  return bounds_;
}

void DesktopRootWindowHostXWalk::SetBounds(const gfx::Rect& bounds) {
  bool origin_changed = bounds_.origin() != bounds.origin();
  bool size_changed = bounds_.size() != bounds.size();
  XWindowChanges changes = {0};
  unsigned value_mask = 0;

  if (size_changed) {
    // X11 will send an XError at our process if have a 0 sized window.
    DCHECK_GT(bounds.width(), 0);
    DCHECK_GT(bounds.height(), 0);

    changes.width = bounds.width();
    changes.height = bounds.height();
    value_mask |= CWHeight | CWWidth;
  }

  if (origin_changed) {
    changes.x = bounds.x();
    changes.y = bounds.y();
    value_mask |= CWX | CWY;
  }
  if (value_mask)
    XConfigureWindow(xdisplay_, xwindow_, value_mask, &changes);

  // Assume that the resize will go through as requested, which should be the
  // case if we're running without a window manager.  If there's a window
  // manager, it can modify or ignore the request, but (per ICCCM) we'll get a
  // (possibly synthetic) ConfigureNotify about the actual size and correct
  // |bounds_| later.
  bounds_ = bounds;

  if (origin_changed)
    native_widget_delegate_->AsWidget()->OnNativeWidgetMove();
  if (size_changed)
    root_window_host_delegate_->OnHostResized(bounds.size());
  else
    root_window_host_delegate_->OnHostPaint(gfx::Rect(bounds.size()));
}

gfx::Insets DesktopRootWindowHostXWalk::GetInsets() const {
  return gfx::Insets();
}

void DesktopRootWindowHostXWalk::SetInsets(const gfx::Insets& insets) {
}

gfx::Point DesktopRootWindowHostXWalk::GetLocationOnNativeScreen() const {
  return bounds_.origin();
}

void DesktopRootWindowHostXWalk::SetCapture() {
  // This is vaguely based on the old NativeWidgetGtk implementation.
  //
  // X11's XPointerGrab() shouldn't be used for everything; it doesn't map
  // cleanly to Windows' SetCapture(). GTK only provides a separate concept of
  // a grab that wasn't the X11 pointer grab, but was instead a manual
  // redirection of the event. (You need to drop into GDK if you want to
  // perform a raw X11 grab).

  if (g_current_capture)
    g_current_capture->OnCaptureReleased();

  g_current_capture = this;

  // TODO(erg): In addition to the above, NativeWidgetGtk performs a full X
  // pointer grab when our NativeWidget is of type Menu. However, things work
  // without it. Clicking inside a chrome window causes a release capture, and
  // clicking outside causes an activation change. Since previous attempts at
  // using XPointerGrab() to implement this have locked my X server, I'm going
  // to skip this for now.
}

void DesktopRootWindowHostXWalk::ReleaseCapture() {
  if (g_current_capture)
    g_current_capture->OnCaptureReleased();
}

void DesktopRootWindowHostXWalk::SetCursor(gfx::NativeCursor cursor) {
  XDefineCursor(xdisplay_, xwindow_, cursor.platform());
}

bool DesktopRootWindowHostXWalk::QueryMouseLocation(
    gfx::Point* location_return) {
  aura::client::CursorClient* cursor_client =
      aura::client::GetCursorClient(GetRootWindow());
  if (cursor_client && !cursor_client->IsMouseEventsEnabled()) {
    *location_return = gfx::Point(0, 0);
    return false;
  }

  ::Window root_return, child_return;
  int root_x_return, root_y_return, win_x_return, win_y_return;
  unsigned int mask_return;
  XQueryPointer(xdisplay_,
                xwindow_,
                &root_return,
                &child_return,
                &root_x_return, &root_y_return,
                &win_x_return, &win_y_return,
                &mask_return);
  *location_return = gfx::Point(
      std::max(0, std::min(bounds_.width(), win_x_return)),
      std::max(0, std::min(bounds_.height(), win_y_return)));
  return (win_x_return >= 0 && win_x_return < bounds_.width() &&
          win_y_return >= 0 && win_y_return < bounds_.height());
}

bool DesktopRootWindowHostXWalk::ConfineCursorToRootWindow() {
  NOTIMPLEMENTED();
  return false;
}

void DesktopRootWindowHostXWalk::UnConfineCursor() {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::OnCursorVisibilityChanged(bool show) {
  // TODO(erg): Conditional on us enabling touch on desktop linux builds, do
  // the same tap-to-click disabling here that chromeos does.
}

void DesktopRootWindowHostXWalk::MoveCursorTo(const gfx::Point& location) {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostXWalk::SetFocusWhenShown(bool focus_when_shown) {
  static const char* k_NET_WM_USER_TIME = "_NET_WM_USER_TIME";
  focus_when_shown_ = focus_when_shown;
  if (IsWindowManagerPresent() && !focus_when_shown_) {
    ui::SetIntProperty(xwindow_,
                       k_NET_WM_USER_TIME,
                       k_NET_WM_USER_TIME,
                       0);
  }
}

bool DesktopRootWindowHostXWalk::CopyAreaToSkCanvas(
    const gfx::Rect& source_bounds,
    const gfx::Point& dest_offset,
    SkCanvas* canvas) {
  NOTIMPLEMENTED();
  return false;
}

void DesktopRootWindowHostXWalk::PostNativeEvent(
    const base::NativeEvent& native_event) {
  DCHECK(xwindow_);
  DCHECK(xdisplay_);
  XEvent xevent = *native_event;
  xevent.xany.display = xdisplay_;
  xevent.xany.window = xwindow_;

  switch (xevent.type) {
    case EnterNotify:
    case LeaveNotify:
    case MotionNotify:
    case KeyPress:
    case KeyRelease:
    case ButtonPress:
    case ButtonRelease: {
      // The fields used below are in the same place for all of events
      // above. Using xmotion from XEvent's unions to avoid repeating
      // the code.
      xevent.xmotion.root = x_root_window_;
      xevent.xmotion.time = CurrentTime;

      gfx::Point point(xevent.xmotion.x, xevent.xmotion.y);
      root_window_->ConvertPointToNativeScreen(&point);
      xevent.xmotion.x_root = point.x();
      xevent.xmotion.y_root = point.y();
    }
    default:
      break;
  }
  XSendEvent(xdisplay_, xwindow_, False, 0, &xevent);
}

void DesktopRootWindowHostXWalk::OnDeviceScaleFactorChanged(
    float device_scale_factor) {
}

void DesktopRootWindowHostXWalk::PrepareForShutdown() {
}

////////////////////////////////////////////////////////////////////////////////
// ui::DesktopSelectionProviderAuraX11 implementation:

void DesktopRootWindowHostXWalk::SetDropHandler(
    ui::OSExchangeDataProviderAuraX11* handler) {
  if (handler) {
    DCHECK(!drop_handler_);
    drop_handler_ = handler;
  } else {
    DCHECK(drop_handler_);
    drop_handler_ = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostXWalk, MessageLoop::Dispatcher implementation:

bool DesktopRootWindowHostXWalk::Dispatch(const base::NativeEvent& event) {
  XEvent* xev = event;

  // May want to factor CheckXEventForConsistency(xev); into a common location
  // since it is called here.
  switch (xev->type) {
    case Expose: {
      gfx::Rect damage_rect(xev->xexpose.x, xev->xexpose.y,
                            xev->xexpose.width, xev->xexpose.height);
      root_window_host_delegate_->OnHostPaint(damage_rect);
      break;
    }
    case KeyPress: {
      ui::KeyEvent keydown_event(xev, false);
      root_window_host_delegate_->OnHostKeyEvent(&keydown_event);
      break;
    }
    case KeyRelease: {
      ui::KeyEvent keyup_event(xev, false);
      root_window_host_delegate_->OnHostKeyEvent(&keyup_event);
      break;
    }
    case ButtonPress: {
      if (static_cast<int>(xev->xbutton.button) == kBackMouseButton ||
          static_cast<int>(xev->xbutton.button) == kForwardMouseButton) {
        aura::client::UserActionClient* gesture_client =
            aura::client::GetUserActionClient(root_window_);
        if (gesture_client) {
          gesture_client->OnUserAction(
              static_cast<int>(xev->xbutton.button) == kBackMouseButton ?
              aura::client::UserActionClient::BACK :
              aura::client::UserActionClient::FORWARD);
        }
        break;
      }
    }  // fallthrough
    case ButtonRelease: {
      ui::MouseEvent mouseev(xev);
      DispatchMouseEvent(&mouseev);
      break;
    }
    case FocusOut:
      if (xev->xfocus.mode != NotifyGrab) {
        ReleaseCapture();
        root_window_host_delegate_->OnHostLostWindowCapture();
      } else {
        root_window_host_delegate_->OnHostLostMouseGrab();
      }
      break;
    case ConfigureNotify: {
      DCHECK_EQ(xwindow_, xev->xconfigure.window);
      DCHECK_EQ(xwindow_, xev->xconfigure.event);
      // It's possible that the X window may be resized by some other means than
      // from within aura (e.g. the X window manager can change the size). Make
      // sure the root window size is maintained properly.
      int translated_x = xev->xconfigure.x;
      int translated_y = xev->xconfigure.y;
      if (!xev->xconfigure.send_event && !xev->xconfigure.override_redirect) {
        Window unused;
        XTranslateCoordinates(xdisplay_, xwindow_, x_root_window_,
            0, 0, &translated_x, &translated_y, &unused);
      }
      gfx::Rect bounds(translated_x, translated_y,
                       xev->xconfigure.width, xev->xconfigure.height);
      bool size_changed = bounds_.size() != bounds.size();
      bool origin_changed = bounds_.origin() != bounds.origin();
      bounds_ = bounds;
      if (size_changed)
        root_window_host_delegate_->OnHostResized(bounds.size());
      if (origin_changed)
        root_window_host_delegate_->OnHostMoved(bounds_.origin());
      break;
    }
    case GenericEvent: {
      ui::TouchFactory* factory = ui::TouchFactory::GetInstance();
      if (!factory->ShouldProcessXI2Event(xev))
        break;

      ui::EventType type = ui::EventTypeFromNative(xev);
      XEvent last_event;
      int num_coalesced = 0;

      switch (type) {
        case ui::ET_TOUCH_MOVED:
        case ui::ET_TOUCH_PRESSED:
        case ui::ET_TOUCH_RELEASED: {
          ui::TouchEvent touchev(xev);
          root_window_host_delegate_->OnHostTouchEvent(&touchev);
          break;
        }
        case ui::ET_MOUSE_MOVED:
        case ui::ET_MOUSE_DRAGGED:
        case ui::ET_MOUSE_PRESSED:
        case ui::ET_MOUSE_RELEASED:
        case ui::ET_MOUSE_ENTERED:
        case ui::ET_MOUSE_EXITED: {
          if (type == ui::ET_MOUSE_MOVED || type == ui::ET_MOUSE_DRAGGED) {
            // If this is a motion event, we want to coalesce all pending motion
            // events that are at the top of the queue.
            num_coalesced = ui::CoalescePendingMotionEvents(xev, &last_event);
            if (num_coalesced > 0)
              xev = &last_event;
          } else if (type == ui::ET_MOUSE_PRESSED) {
            XIDeviceEvent* xievent =
                static_cast<XIDeviceEvent*>(xev->xcookie.data);
            int button = xievent->detail;
            if (button == kBackMouseButton || button == kForwardMouseButton) {
              aura::client::UserActionClient* gesture_client =
                  aura::client::GetUserActionClient(
                      root_window_host_delegate_->AsRootWindow());
              if (gesture_client) {
                bool reverse_direction =
                    ui::IsTouchpadEvent(xev) && ui::IsNaturalScrollEnabled();
                gesture_client->OnUserAction(
                    (button == kBackMouseButton && !reverse_direction) ||
                    (button == kForwardMouseButton && reverse_direction) ?
                    aura::client::UserActionClient::BACK :
                    aura::client::UserActionClient::FORWARD);
              }
              break;
            }
          }
          ui::MouseEvent mouseev(xev);
          DispatchMouseEvent(&mouseev);
          break;
        }
        case ui::ET_MOUSEWHEEL: {
          ui::MouseWheelEvent mouseev(xev);
          DispatchMouseEvent(&mouseev);
          break;
        }
        case ui::ET_SCROLL_FLING_START:
        case ui::ET_SCROLL_FLING_CANCEL:
        case ui::ET_SCROLL: {
          ui::ScrollEvent scrollev(xev);
          root_window_host_delegate_->OnHostScrollEvent(&scrollev);
          break;
        }
        case ui::ET_UNKNOWN:
          break;
        default:
          NOTREACHED();
      }

      // If we coalesced an event we need to free its cookie.
      if (num_coalesced > 0)
        XFreeEventData(xev->xgeneric.display, &last_event.xcookie);
      break;
    }
    case MapNotify: {
      // If there's no window manager running, we need to assign the X input
      // focus to our host window.
      if (!IsWindowManagerPresent() && focus_when_shown_)
        XSetInputFocus(xdisplay_, xwindow_, RevertToNone, CurrentTime);
      break;
    }
    case ClientMessage: {
      Atom message_type = xev->xclient.message_type;
      if (message_type == atom_cache_.GetAtom("WM_PROTOCOLS")) {
        Atom protocol = static_cast<Atom>(xev->xclient.data.l[0]);
        if (protocol == atom_cache_.GetAtom("WM_DELETE_WINDOW")) {
          // We have received a close message from the window manager.
          root_window_->OnRootWindowHostCloseRequested();
        } else if (protocol == atom_cache_.GetAtom("_NET_WM_PING")) {
          XEvent reply_event = *xev;
          reply_event.xclient.window = x_root_window_;

          XSendEvent(xdisplay_,
                     reply_event.xclient.window,
                     False,
                     SubstructureRedirectMask | SubstructureNotifyMask,
                     &reply_event);
        }
      } else if (message_type == atom_cache_.GetAtom("XdndEnter")) {
        drag_drop_client_->OnXdndEnter(xev->xclient);
      } else if (message_type == atom_cache_.GetAtom("XdndLeave")) {
        drag_drop_client_->OnXdndLeave(xev->xclient);
      } else if (message_type == atom_cache_.GetAtom("XdndPosition")) {
        drag_drop_client_->OnXdndPosition(xev->xclient);
      } else if (message_type == atom_cache_.GetAtom("XdndStatus")) {
        drag_drop_client_->OnXdndStatus(xev->xclient);
      } else if (message_type == atom_cache_.GetAtom("XdndFinished")) {
        drag_drop_client_->OnXdndFinished(xev->xclient);
      } else if (message_type == atom_cache_.GetAtom("XdndDrop")) {
        drag_drop_client_->OnXdndDrop(xev->xclient);
      }
      break;
    }
    case MappingNotify: {
      switch (xev->xmapping.request) {
        case MappingModifier:
        case MappingKeyboard:
          XRefreshKeyboardMapping(&xev->xmapping);
          root_window_->OnKeyboardMappingChanged();
          break;
        case MappingPointer:
          ui::UpdateButtonMap();
          break;
        default:
          NOTIMPLEMENTED() << " Unknown request: " << xev->xmapping.request;
          break;
      }
      break;
    }
    case MotionNotify: {
      // Discard all but the most recent motion event that targets the same
      // window with unchanged state.
      XEvent last_event;
      while (XPending(xev->xany.display)) {
        XEvent next_event;
        XPeekEvent(xev->xany.display, &next_event);
        if (next_event.type == MotionNotify &&
            next_event.xmotion.window == xev->xmotion.window &&
            next_event.xmotion.subwindow == xev->xmotion.subwindow &&
            next_event.xmotion.state == xev->xmotion.state) {
          XNextEvent(xev->xany.display, &last_event);
          xev = &last_event;
        } else {
          break;
        }
      }

      ui::MouseEvent mouseev(xev);
      DispatchMouseEvent(&mouseev);
      break;
    }
    case PropertyNotify: {
      // Get our new window property state if the WM has told us its changed.
      ::Atom state = atom_cache_.GetAtom("_NET_WM_STATE");

      std::vector< ::Atom> atom_list;
      if (xev->xproperty.atom == state &&
          ui::GetAtomArrayProperty(xwindow_, "_NET_WM_STATE", &atom_list)) {
        window_properties_.clear();
        std::copy(atom_list.begin(), atom_list.end(),
                  inserter(window_properties_, window_properties_.begin()));

        // Now that we have different window properties, we may need to
        // relayout the window. (The windows code doesn't need this because
        // their window change is synchronous.)
        //
        // TODO(erg): While this does work, there's a quick flash showing the
        // tabstrip/toolbar/etc. when going into fullscreen mode before hiding
        // those parts of the UI because we receive the sizing event from the
        // window manager before we receive the event that changes the
        // fullscreen state. Unsure what to do about that.
        Widget* widget = native_widget_delegate_->AsWidget();
        NonClientView* non_client_view = widget->non_client_view();
        // non_client_view may be NULL, especially during creation.
        if (non_client_view) {
          non_client_view->client_view()->InvalidateLayout();
          non_client_view->InvalidateLayout();
        }
        widget->GetRootView()->Layout();
      }
      break;
    }
    case SelectionNotify: {
      if (drop_handler_)
        drop_handler_->OnSelectionNotify(xev->xselection);
      break;
    }
  }
  return true;
}

void DesktopRootWindowHostXWalk::OnRootWindowHostCloseRequested(
    const aura::RootWindow* root) {
  DCHECK(root == root_window_);
  Close();
}

// static
DesktopRootWindowHost* DesktopRootWindowHostXWalk::Create(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura,
    const gfx::Rect& initial_bounds) {
  return new DesktopRootWindowHostXWalk(native_widget_delegate,
                                        desktop_native_widget_aura,
                                        initial_bounds);
}

}  // namespace views
