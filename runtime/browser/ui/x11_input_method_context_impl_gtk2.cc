// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/x11_input_method_context_impl_gtk2.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <string>

#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/ime/composition_text.h"
#include "ui/base/ime/composition_text_util_pango.h"
#include "ui/base/ime/text_input_client.h"
#include "ui/events/event.h"
#include "ui/events/keycodes/keyboard_code_conversion_x.h"
#include "ui/gfx/x/x11_types.h"

namespace xwalk {

X11InputMethodContextImplGtk2::X11InputMethodContextImplGtk2(
    ui::LinuxInputMethodContextDelegate* delegate)
    : delegate_(delegate),
      gtk_context_simple_(NULL),
      gtk_multicontext_(NULL),
      gtk_context_(NULL),
      gdk_last_set_client_window_(NULL) {
  CHECK(delegate_);

  ResetXModifierKeycodesCache();

  gtk_context_simple_ = gtk_im_context_simple_new();
  gtk_multicontext_ = gtk_im_multicontext_new();

  GtkIMContext* contexts[] = {gtk_context_simple_, gtk_multicontext_};
  for (size_t i = 0; i < arraysize(contexts); ++i) {
    g_signal_connect(contexts[i], "commit",
                     G_CALLBACK(OnCommitThunk), this);
    g_signal_connect(contexts[i], "preedit-changed",
                     G_CALLBACK(OnPreeditChangedThunk), this);
    g_signal_connect(contexts[i], "preedit-end",
                     G_CALLBACK(OnPreeditEndThunk), this);
    g_signal_connect(contexts[i], "preedit-start",
                     G_CALLBACK(OnPreeditStartThunk), this);
    // TODO(yukishiino): Handle operations on surrounding text.
    // "delete-surrounding" and "retrieve-surrounding" signals should be
    // handled.
  }
}

X11InputMethodContextImplGtk2::~X11InputMethodContextImplGtk2() {
  gtk_context_ = NULL;
  if (gtk_context_simple_) {
    g_object_unref(gtk_context_simple_);
    gtk_context_simple_ = NULL;
  }
  if (gtk_multicontext_) {
    g_object_unref(gtk_multicontext_);
    gtk_multicontext_ = NULL;
  }
}

// Overriden from ui::LinuxInputMethodContext

bool X11InputMethodContextImplGtk2::DispatchKeyEvent(
    const ui::KeyEvent& key_event) {
  if (!key_event.HasNativeEvent())
    return false;

  // The caller must call Focus() first.
  if (!gtk_context_)
    return false;

  // Translate a XKeyEvent to a GdkEventKey.
  GdkEvent* event = GdkEventFromNativeEvent(key_event.native_event());
  if (!event) {
    LOG(ERROR) << "Cannot translate a XKeyEvent to a GdkEvent.";
    return false;
  }

  // Set the client window and cursor location.
  if (event->key.window != gdk_last_set_client_window_) {
    gtk_im_context_set_client_window(gtk_context_, event->key.window);
    gdk_last_set_client_window_ = event->key.window;
  }
  // Convert the last known caret bounds relative to the screen coordinates
  // to a GdkRectangle relative to the client window.
  gint x = 0;
  gint y = 0;
  gdk_window_get_origin(event->key.window, &x, &y);
  GdkRectangle rect = {last_caret_bounds_.x() - x,
                       last_caret_bounds_.y() - y,
                       last_caret_bounds_.width(),
                       last_caret_bounds_.height()};
  gtk_im_context_set_cursor_location(gtk_context_, &rect);

  // Let an IME handle the key event.
  commit_signal_trap_.StartTrap(event->key.keyval);
  const gboolean handled = gtk_im_context_filter_keypress(gtk_context_,
                                                          &event->key);
  commit_signal_trap_.StopTrap();
  gdk_event_free(event);

  return handled && !commit_signal_trap_.IsSignalCaught();
}

void X11InputMethodContextImplGtk2::Reset() {
  // Reset all the states of the context, not only preedit, caret but also
  // focus.
  gtk_context_ = NULL;
  gtk_im_context_reset(gtk_context_simple_);
  gtk_im_context_reset(gtk_multicontext_);
  gtk_im_context_focus_out(gtk_context_simple_);
  gtk_im_context_focus_out(gtk_multicontext_);
  gdk_last_set_client_window_ = NULL;
}

void X11InputMethodContextImplGtk2::OnTextInputTypeChanged(
    ui::TextInputType text_input_type) {
  switch (text_input_type) {
    case ui::TEXT_INPUT_TYPE_NONE:
    case ui::TEXT_INPUT_TYPE_PASSWORD:
      gtk_context_ = gtk_context_simple_;
      break;
    default:
      gtk_context_ = gtk_multicontext_;
  }
  gtk_im_context_focus_in(gtk_context_);
}

void X11InputMethodContextImplGtk2::OnCaretBoundsChanged(
    const gfx::Rect& caret_bounds) {
  // Remember the caret bounds so that we can set the cursor location later.
  // gtk_im_context_set_cursor_location() takes the location relative to the
  // client window, which is unknown at this point.  So we'll call
  // gtk_im_context_set_cursor_location() later in ProcessKeyEvent() where
  // (and only where) we know the client window.
  last_caret_bounds_ = caret_bounds;
}

// private:

void X11InputMethodContextImplGtk2::ResetXModifierKeycodesCache() {
  modifier_keycodes_.clear();
  meta_keycodes_.clear();
  super_keycodes_.clear();
  hyper_keycodes_.clear();

  Display* display = gfx::GetXDisplay();
  const XModifierKeymap* modmap = XGetModifierMapping(display);
  int min_keycode = 0;
  int max_keycode = 0;
  int keysyms_per_keycode = 1;
  XDisplayKeycodes(display, &min_keycode, &max_keycode);
  const KeySym* keysyms = XGetKeyboardMapping(
      display, min_keycode, max_keycode - min_keycode + 1,
      &keysyms_per_keycode);
  for (int i = 0; i < 8 * modmap->max_keypermod; ++i) {
    const int keycode = modmap->modifiermap[i];
    if (!keycode)
      continue;
    modifier_keycodes_.insert(keycode);

    if (!keysyms)
      continue;
    for (int j = 0; j < keysyms_per_keycode; ++j) {
      switch (keysyms[(keycode - min_keycode) * keysyms_per_keycode + j]) {
        case XK_Meta_L:
        case XK_Meta_R:
          meta_keycodes_.push_back(keycode);
          break;
        case XK_Super_L:
        case XK_Super_R:
          super_keycodes_.push_back(keycode);
          break;
        case XK_Hyper_L:
        case XK_Hyper_R:
          hyper_keycodes_.push_back(keycode);
          break;
      }
    }
  }
  XFree(const_cast<KeySym*>(keysyms));
  XFreeModifiermap(const_cast<XModifierKeymap*>(modmap));
}

GdkEvent* X11InputMethodContextImplGtk2::GdkEventFromNativeEvent(
    const base::NativeEvent& native_event) {
  XEvent xkeyevent;
  if (native_event->type == GenericEvent) {
    // If this is an XI2 key event, build a matching core X event, to avoid
    // having two cases for every use.
    ui::InitXKeyEventFromXIDeviceEvent(*native_event, &xkeyevent);
  } else {
    DCHECK(native_event->type == KeyPress || native_event->type == KeyRelease);
    xkeyevent.xkey = native_event->xkey;
  }
  XKeyEvent& xkey = xkeyevent.xkey;

  // Get a GdkDisplay.
  GdkDisplay* display = gdk_x11_lookup_xdisplay(xkey.display);
  if (!display) {
    // Fall back to the default display.
    display = gdk_display_get_default();
  }
  if (!display) {
    LOG(ERROR) << "Cannot get a GdkDisplay for a key event.";
    return NULL;
  }
  // Get a keysym and group.
  KeySym keysym = NoSymbol;
  guint8 keyboard_group = 0;
  XLookupString(&xkey, NULL, 0, &keysym, NULL);
  GdkKeymap* keymap = gdk_keymap_get_for_display(display);
  GdkKeymapKey* keys = NULL;
  guint* keyvals = NULL;
  gint n_entries = 0;
  if (keymap &&
      gdk_keymap_get_entries_for_keycode(keymap, xkey.keycode,
                                         &keys, &keyvals, &n_entries)) {
    for (gint i = 0; i < n_entries; ++i) {
      if (keyvals[i] == keysym) {
        keyboard_group = keys[i].group;
        break;
      }
    }
  }
  g_free(keys);
  keys = NULL;
  g_free(keyvals);
  keyvals = NULL;
  // Get a GdkWindow.
#if GTK_CHECK_VERSION(2, 24, 0)
  GdkWindow* window = gdk_x11_window_lookup_for_display(display, xkey.window);
#else
  GdkWindow* window = gdk_window_lookup_for_display(display, xkey.window);
#endif
  if (window)
    g_object_ref(window);
  else
#if GTK_CHECK_VERSION(2, 24, 0)
    window = gdk_x11_window_foreign_new_for_display(display, xkey.window);
#else
    window = gdk_window_foreign_new_for_display(display, xkey.window);
#endif
  if (!window) {
    LOG(ERROR) << "Cannot get a GdkWindow for a key event.";
    return NULL;
  }

  // Create a GdkEvent.
  GdkEventType event_type = xkey.type == KeyPress ?
                            GDK_KEY_PRESS : GDK_KEY_RELEASE;
  GdkEvent* event = gdk_event_new(event_type);
  event->key.type = event_type;
  event->key.window = window;
  // GdkEventKey and XKeyEvent share the same definition for time and state.
  event->key.send_event = xkey.send_event;
  event->key.time = xkey.time;
  event->key.state = xkey.state;
  event->key.keyval = keysym;
  event->key.length = 0;
  event->key.string = NULL;
  event->key.hardware_keycode = xkey.keycode;
  event->key.group = keyboard_group;
  event->key.is_modifier = IsKeycodeModifierKey(xkey.keycode);

  char keybits[32] = {0};
  XQueryKeymap(xkey.display, keybits);
  if (IsAnyOfKeycodesPressed(meta_keycodes_, keybits, sizeof keybits * 8))
    event->key.state |= GDK_META_MASK;
  if (IsAnyOfKeycodesPressed(super_keycodes_, keybits, sizeof keybits * 8))
    event->key.state |= GDK_SUPER_MASK;
  if (IsAnyOfKeycodesPressed(hyper_keycodes_, keybits, sizeof keybits * 8))
    event->key.state |= GDK_HYPER_MASK;

  return event;
}

bool X11InputMethodContextImplGtk2::IsKeycodeModifierKey(
    unsigned int keycode) const {
  return modifier_keycodes_.find(keycode) != modifier_keycodes_.end();
}

bool X11InputMethodContextImplGtk2::IsAnyOfKeycodesPressed(
    const std::vector<int>& keycodes,
    const char* keybits,
    int num_keys) const {
  for (size_t i = 0; i < keycodes.size(); ++i) {
    const int keycode = keycodes[i];
    if (keycode < 0 || num_keys <= keycode)
      continue;
    if (keybits[keycode / 8] & 1 << (keycode % 8))
      return true;
  }
  return false;
}

// GtkIMContext event handlers.

void X11InputMethodContextImplGtk2::OnCommit(GtkIMContext* context,
                                             gchar* text) {
  if (context != gtk_context_)
    return;

  const base::string16& text_in_utf16 = base::UTF8ToUTF16(text);
  // If an underlying IME is emitting the "commit" signal to insert a character
  // for a direct input key event, ignores the insertion of the character at
  // this point, because we have to call DispatchKeyEventPostIME() for direct
  // input key events.  DispatchKeyEvent() takes care of the trapped character
  // and calls DispatchKeyEventPostIME().
  if (commit_signal_trap_.Trap(text_in_utf16))
    return;

  delegate_->OnCommit(text_in_utf16);
}

void X11InputMethodContextImplGtk2::OnPreeditChanged(GtkIMContext* context) {
  if (context != gtk_context_)
    return;

  gchar* str = NULL;
  PangoAttrList* attrs = NULL;
  gint cursor_pos = 0;
  gtk_im_context_get_preedit_string(context, &str, &attrs, &cursor_pos);
  ui::CompositionText composition_text;
  ui::ExtractCompositionTextFromGtkPreedit(str, attrs, cursor_pos,
                                           &composition_text);
  g_free(str);
  pango_attr_list_unref(attrs);

  delegate_->OnPreeditChanged(composition_text);
}

void X11InputMethodContextImplGtk2::OnPreeditEnd(GtkIMContext* context) {
  if (context != gtk_context_)
    return;

  delegate_->OnPreeditEnd();
}

void X11InputMethodContextImplGtk2::OnPreeditStart(GtkIMContext* context) {
  if (context != gtk_context_)
    return;

  delegate_->OnPreeditStart();
}

// GtkCommitSignalTrap

X11InputMethodContextImplGtk2::GtkCommitSignalTrap::GtkCommitSignalTrap()
    : is_trap_enabled_(false),
#if GTK_CHECK_VERSION (2, 22, 0)
      gdk_event_key_keyval_(GDK_KEY_VoidSymbol),
#else
      gdk_event_key_keyval_(GDK_VoidSymbol),
#endif
      is_signal_caught_(false) {}

void X11InputMethodContextImplGtk2::GtkCommitSignalTrap::StartTrap(
    guint keyval) {
  is_signal_caught_ = false;
  gdk_event_key_keyval_ = keyval;
  is_trap_enabled_ = true;
}

void X11InputMethodContextImplGtk2::GtkCommitSignalTrap::StopTrap() {
  is_trap_enabled_ = false;
}

bool X11InputMethodContextImplGtk2::GtkCommitSignalTrap::Trap(
    const base::string16& text) {
  DCHECK(!is_signal_caught_);
  if (is_trap_enabled_ &&
      text.length() == 1 &&
      text[0] == gdk_keyval_to_unicode(gdk_event_key_keyval_)) {
    is_signal_caught_ = true;
    return true;
  } else {
    return false;
  }
}

}  // namespace xwalk
