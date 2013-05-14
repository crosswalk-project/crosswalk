// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/ui/native_app_window_gtk.h"

#include <gdk/gdk.h>

#include "base/utf_string_conversions.h"
#include "cameo/src/runtime/browser/runtime.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/renderer_preferences.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/x/active_window_watcher_x.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/gtk_util.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/skia_utils_gtk.h"

namespace cameo {

namespace {

// Dividing GTK's cursor blink cycle time (in milliseconds) by this value yields
// an appropriate value for content::RendererPreferences::caret_blink_interval.
// This matches the logic in the WebKit GTK port.
const double kGtkCursorBlinkCycleFactor = 2000.0;

}  // namespace

NativeAppWindowGtk::NativeAppWindowGtk(const NativeAppWindow::CreateParams& params)
    : runtime_(params.runtime),
      minimum_size_(params.minimum_size),
      maximum_size_(params.maximum_size),
      is_fullscreen_(false),
      resizable_(params.resizable),
      is_active_(false) {
  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

  vbox_ = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox_);
  gtk_container_add(GTK_CONTAINER(window_), vbox_);

  gfx::NativeView native_view =
     runtime_->web_contents()->GetView()->GetNativeView();
  gtk_widget_show(native_view);
  gtk_container_add(GTK_CONTAINER(vbox_), native_view);

  gfx::Size size = params.bounds.size();
  gint width = static_cast<gint>(size.width());
  gint height = static_cast<gint>(size.height());

  gtk_window_set_default_size(window_, width, height);

  SetMaximumSize(window_, maximum_size_);
  SetMinimumSize(window_, minimum_size_);
  SetResizable(window_, resizable_);

   // In some (older) versions of compiz, raising top-level windows when they
  // are partially off-screen causes them to get snapped back on screen, not
  // always even on the current virtual desktop.  If we are running under
  // compiz, suppress such raises, as they are not necessary in compiz anyway.
  if (ui::GuessWindowManager() == ui::WM_COMPIZ)
    suppress_window_raise_ = true;

  g_signal_connect(window_, "destroy",
                   G_CALLBACK(OnWindowDestroyedThunk), this);
  g_signal_connect(window_, "window-state-event",
                   G_CALLBACK(OnWindowStateThunk), this);
  g_signal_connect(window_, "delete-event",
                   G_CALLBACK(OnWindowDeleteEventThunk), this);

  SetWebKitColorStyle(window_);
  gtk_widget_realize(GTK_WIDGET(window_));

  // Center the window in the screen.
  gtk_window_set_position(window_, GTK_WIN_POS_CENTER);
  ui::ActiveWindowWatcherX::AddObserver(this);
}

NativeAppWindowGtk::~NativeAppWindowGtk() {
  ui::ActiveWindowWatcherX::RemoveObserver(this);
}

void NativeAppWindowGtk::UpdateIcon() {
  gfx::Image app_icon = runtime_->app_icon();
  gtk_window_set_icon(window_, app_icon.ToGdkPixbuf());
}

void NativeAppWindowGtk::UpdateTitle(const string16& title) {
  std::string title_utf8 = UTF16ToUTF8(title);
  gtk_window_set_title(GTK_WINDOW(window_), title_utf8.c_str());
}

gfx::Rect NativeAppWindowGtk::GetRestoredBounds() const {
  // TODO(hmin): Need to implement get restored bounds of native window.
  return GetBounds();
}

gfx::Rect NativeAppWindowGtk::GetBounds() const {
  GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(window_));

  GdkRectangle rect;
  gdk_window_get_frame_extents(gdk_window, &rect);

  return gfx::Rect(rect.x, rect.y, rect.width, rect.height);
}

void NativeAppWindowGtk::SetBounds(const gfx::Rect& bounds) {
  gint x = static_cast<gint>(bounds.x());
  gint y = static_cast<gint>(bounds.y());
  gint width = static_cast<gint>(bounds.width());
  gint height = static_cast<gint>(bounds.height());

  gtk_window_move(window_, x, y);
  SetWindowSize(window_, gfx::Size(width, height));
}

void NativeAppWindowGtk::Focus() {
  gtk_window_present(window_);
}

void NativeAppWindowGtk::Show() {
  gtk_widget_show(GTK_WIDGET(window_));
}

void NativeAppWindowGtk::Hide() {
  gtk_widget_hide(GTK_WIDGET(window_));
}

void NativeAppWindowGtk::Maximize() {
  gtk_window_maximize(window_);
}

void NativeAppWindowGtk::Minimize() {
  gtk_window_iconify(window_);
}

void NativeAppWindowGtk::SetFullscreen(bool fullscreen) {
  if (is_fullscreen_ == fullscreen)
    return;

  is_fullscreen_ = fullscreen;
  if (fullscreen)
    gtk_window_fullscreen(window_);
  else
    gtk_window_unfullscreen(window_);
}

void NativeAppWindowGtk::Restore() {
  if (IsMaximized())
    gtk_window_unmaximize(window_);
  else if (IsMinimized())
    gtk_window_deiconify(window_);
}

void NativeAppWindowGtk::FlashFrame(bool flash) {
  gtk_window_set_urgency_hint(window_, flash);
}

void NativeAppWindowGtk::ActiveWindowChanged(GdkWindow* active_window) {
  // Do nothing if we're in the process of closing the browser window.
  if (!window_)
    return;

  is_active_ = gtk_widget_get_window(GTK_WIDGET(window_)) == active_window;
}

void NativeAppWindowGtk::Close() {
  gtk_widget_destroy(GTK_WIDGET(window_));
}

bool NativeAppWindowGtk::IsActive() const {
  if (ui::ActiveWindowWatcherX::WMSupportsActivation())
    return is_active_;

  // This still works even though we don't get the activation notification.
  return gtk_window_is_active(window_);
}

bool NativeAppWindowGtk::IsMaximized() const {
  return (state_ & GDK_WINDOW_STATE_MAXIMIZED);
}

bool NativeAppWindowGtk::IsMinimized() const {
  return (state_ & GDK_WINDOW_STATE_ICONIFIED);
}

bool NativeAppWindowGtk::IsFullscreen() const {
  return (state_ & GDK_WINDOW_STATE_FULLSCREEN);
}

void NativeAppWindowGtk::SetWebKitColorStyle(GtkWindow* window) {
  // Set WebKit's styles according to current GTK theme.
  content::RendererPreferences* prefs =
      runtime_->web_contents()->GetMutableRendererPrefs();
  GtkStyle* frame_style = gtk_rc_get_style(GTK_WIDGET(window));
  prefs->focus_ring_color =
      gfx::GdkColorToSkColor(frame_style->bg[GTK_STATE_SELECTED]);
  prefs->thumb_active_color = SkColorSetRGB(244, 244, 244);
  prefs->thumb_inactive_color = SkColorSetRGB(234, 234, 234);
  prefs->track_color = SkColorSetRGB(211, 211, 211);

  const base::TimeDelta cursor_blink_time = gfx::GetCursorBlinkCycle();
  prefs->caret_blink_interval =
      cursor_blink_time.InMilliseconds() ?
      cursor_blink_time.InMilliseconds() / kGtkCursorBlinkCycleFactor :
      0;
}

// Callback for when the main window is destroyed.
gboolean NativeAppWindowGtk::OnWindowDestroyed(GtkWidget* window) {
  runtime_->Close();
  delete this;
  return FALSE;
}

// Window state has changed.
gboolean NativeAppWindowGtk::OnWindowState(GtkWidget* window,
                                           GdkEventWindowState* event) {
  state_ = event->new_window_state;

  if (is_fullscreen_ && !(state_ & GDK_WINDOW_STATE_FULLSCREEN)) {
    is_fullscreen_ = false;
    content::RenderViewHost* rvh = runtime_->web_contents()->GetRenderViewHost();
    if (rvh)
      rvh->ExitFullscreen();
  }
  return FALSE;
}

// Window will be closed.
gboolean NativeAppWindowGtk::OnWindowDeleteEvent(GtkWidget* widget,
                                                 GdkEvent* event) {
  return FALSE;
}

// static
void NativeAppWindowGtk::SetMinimumSize(GtkWindow* window,
    const gfx::Size& size) {
  GdkGeometry geometry = { 0 };
  geometry.min_width = size.width();
  geometry.min_height = size.height();
  int hints = GDK_HINT_POS | GDK_HINT_MIN_SIZE;
  gtk_window_set_geometry_hints(
      window, GTK_WIDGET(window), &geometry, (GdkWindowHints)hints);
}

void NativeAppWindowGtk::SetMaximumSize(GtkWindow* window,
    const gfx::Size& size) {
  GdkGeometry geometry = { 0 };
  geometry.max_width = size.width();
  geometry.max_height = size.height();
  int hints = GDK_HINT_POS | GDK_HINT_MAX_SIZE;
  gtk_window_set_geometry_hints(
      window, GTK_WIDGET(window), &geometry, (GdkWindowHints)hints);
}

void NativeAppWindowGtk::SetResizable(GtkWindow* window, bool resizable) {
  // Should request widget size after setting unresizable, otherwise the
  // window will shrink to a very small size.
  if (resizable == false) {
    gint width, height;
    gtk_window_get_size(window, &width, &height);
    gtk_widget_set_size_request(GTK_WIDGET(window), width, height);
  }

  gtk_window_set_resizable(window, resizable);
}

void NativeAppWindowGtk::SetWindowSize(GtkWindow* window,
    const gfx::Size& size) {
  gfx::Size new_size = size;
  gint current_width = 0;
  gint current_height = 0;
  gtk_window_get_size(window, &current_width, &current_height);
  GdkRectangle size_with_decorations = {0};
  GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(window));
  if (gdk_window) {
    gdk_window_get_frame_extents(gdk_window,
                                 &size_with_decorations);
  }

  if (current_width == size_with_decorations.width &&
      current_height == size_with_decorations.height) {
    // Make sure the window doesn't match any monitor size.  We compare against
    // all monitors because we don't know which monitor the window is going to
    // open on (the WM decides that).
    GdkScreen* screen = gtk_window_get_screen(window);
    gint num_monitors = gdk_screen_get_n_monitors(screen);
    for (gint i = 0; i < num_monitors; ++i) {
      GdkRectangle monitor_size;
      gdk_screen_get_monitor_geometry(screen, i, &monitor_size);
      if (gfx::Size(monitor_size.width, monitor_size.height) == size) {
        gtk_window_resize(window, size.width(), size.height() - 1);
        return;
      }
    }
  } else {
    // gtk_window_resize is the size of the window not including decorations,
    // but we are given the |size| including window decorations.
    if (size_with_decorations.width > current_width) {
      new_size.set_width(size.width() - size_with_decorations.width +
          current_width);
    }
    if (size_with_decorations.height > current_height) {
      new_size.set_height(size.height() - size_with_decorations.height +
          current_height);
    }
  }

  gtk_window_resize(window, new_size.width(), new_size.height());
}

NativeAppWindow* NativeAppWindow::Create(
    const NativeAppWindow::CreateParams& params) {
  return new NativeAppWindowGtk(params);
}

}  // namespace cameo
