// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/keycode_text_conversion.h"

#include <algorithm>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "base/strings/utf_string_conversions.h"
#include "chrome/test/chromedriver/chrome/ui_events.h"
#include "ui/base/x/x11_util.h"
#include "ui/events/keycodes/keyboard_code_conversion_x.h"

namespace {

struct KeyCodeAndXKeyCode {
  ui::KeyboardCode key_code;
  int x_key_code;
};

// Contains a list of keyboard codes, in order, with their corresponding
// X key code. This list is not complete.
// TODO(kkania): Merge this table with the existing one in
// keyboard_code_conversion_x.cc.
KeyCodeAndXKeyCode kKeyCodeToXKeyCode[] = {
  { ui::VKEY_BACK, 22 },
  { ui::VKEY_TAB, 23 },
  { ui::VKEY_RETURN, 36 },
  { ui::VKEY_SHIFT, 50 },
  { ui::VKEY_CONTROL, 37 },
  { ui::VKEY_MENU, 64 },
  { ui::VKEY_CAPITAL, 66 },
  { ui::VKEY_HANGUL, 130 },
  { ui::VKEY_HANJA, 131 },
  { ui::VKEY_ESCAPE, 9 },
  { ui::VKEY_SPACE, 65 },
  { ui::VKEY_PRIOR, 112 },
  { ui::VKEY_NEXT, 117 },
  { ui::VKEY_END, 115 },
  { ui::VKEY_HOME, 110 },
  { ui::VKEY_LEFT, 113 },
  { ui::VKEY_UP, 111 },
  { ui::VKEY_RIGHT, 114 },
  { ui::VKEY_DOWN, 116 },
  { ui::VKEY_INSERT, 118 },
  { ui::VKEY_DELETE, 119 },
  { ui::VKEY_0, 19 },
  { ui::VKEY_1, 10 },
  { ui::VKEY_2, 11 },
  { ui::VKEY_3, 12 },
  { ui::VKEY_4, 13 },
  { ui::VKEY_5, 14 },
  { ui::VKEY_6, 15 },
  { ui::VKEY_7, 16 },
  { ui::VKEY_8, 17 },
  { ui::VKEY_9, 18 },
  { ui::VKEY_A, 38 },
  { ui::VKEY_B, 56 },
  { ui::VKEY_C, 54 },
  { ui::VKEY_D, 40 },
  { ui::VKEY_E, 26 },
  { ui::VKEY_F, 41 },
  { ui::VKEY_G, 42 },
  { ui::VKEY_H, 43 },
  { ui::VKEY_I, 31 },
  { ui::VKEY_J, 44 },
  { ui::VKEY_K, 45 },
  { ui::VKEY_L, 46 },
  { ui::VKEY_M, 58 },
  { ui::VKEY_N, 57 },
  { ui::VKEY_O, 32 },
  { ui::VKEY_P, 33 },
  { ui::VKEY_Q, 24 },
  { ui::VKEY_R, 27 },
  { ui::VKEY_S, 39 },
  { ui::VKEY_T, 28 },
  { ui::VKEY_U, 30 },
  { ui::VKEY_V, 55 },
  { ui::VKEY_W, 25 },
  { ui::VKEY_X, 53 },
  { ui::VKEY_Y, 29 },
  { ui::VKEY_Z, 52 },
  { ui::VKEY_LWIN, 133 },
  { ui::VKEY_NUMPAD0, 90 },
  { ui::VKEY_NUMPAD1, 87 },
  { ui::VKEY_NUMPAD2, 88 },
  { ui::VKEY_NUMPAD3, 89 },
  { ui::VKEY_NUMPAD4, 83 },
  { ui::VKEY_NUMPAD5, 84 },
  { ui::VKEY_NUMPAD6, 85 },
  { ui::VKEY_NUMPAD7, 79 },
  { ui::VKEY_NUMPAD8, 80 },
  { ui::VKEY_NUMPAD9, 81 },
  { ui::VKEY_MULTIPLY, 63 },
  { ui::VKEY_ADD, 86 },
  { ui::VKEY_SUBTRACT, 82 },
  { ui::VKEY_DECIMAL, 129 },
  { ui::VKEY_DIVIDE, 106 },
  { ui::VKEY_F1, 67 },
  { ui::VKEY_F2, 68 },
  { ui::VKEY_F3, 69 },
  { ui::VKEY_F4, 70 },
  { ui::VKEY_F5, 71 },
  { ui::VKEY_F6, 72 },
  { ui::VKEY_F7, 73 },
  { ui::VKEY_F8, 74 },
  { ui::VKEY_F9, 75 },
  { ui::VKEY_F10, 76 },
  { ui::VKEY_F11, 95 },
  { ui::VKEY_F12, 96 },
  { ui::VKEY_NUMLOCK, 77 },
  { ui::VKEY_SCROLL, 78 },
  { ui::VKEY_OEM_1, 47 },
  { ui::VKEY_OEM_PLUS, 21 },
  { ui::VKEY_OEM_COMMA, 59 },
  { ui::VKEY_OEM_MINUS, 20 },
  { ui::VKEY_OEM_PERIOD, 60 },
  { ui::VKEY_OEM_2, 61 },
  { ui::VKEY_OEM_3, 49 },
  { ui::VKEY_OEM_4, 34 },
  { ui::VKEY_OEM_5, 51 },
  { ui::VKEY_OEM_6, 35 },
  { ui::VKEY_OEM_7, 48 }
};

// Uses to compare two KeyCodeAndXKeyCode structs based on their key code.
bool operator<(const KeyCodeAndXKeyCode& a, const KeyCodeAndXKeyCode& b) {
  return a.key_code < b.key_code;
}

// Returns the equivalent X key code for the given key code. Returns -1 if
// no X equivalent was found.
int KeyboardCodeToXKeyCode(ui::KeyboardCode key_code) {
  KeyCodeAndXKeyCode find;
  find.key_code = key_code;
  const KeyCodeAndXKeyCode* found = std::lower_bound(
      kKeyCodeToXKeyCode, kKeyCodeToXKeyCode + arraysize(kKeyCodeToXKeyCode),
      find);
  if (found >= kKeyCodeToXKeyCode + arraysize(kKeyCodeToXKeyCode) ||
      found->key_code != key_code)
    return -1;
  return found->x_key_code;
}

// Gets the X modifier mask (Mod1Mask through Mod5Mask) for the given
// modifier. Only checks the alt, meta, and num lock keys currently.
// Returns true on success.
bool GetXModifierMask(Display* display, int modifier, int* x_modifier) {
  XModifierKeymap* mod_map = XGetModifierMapping(display);
  bool found = false;
  int max_mod_keys = mod_map->max_keypermod;
  for (int mod_index = 0; mod_index <= 8; ++mod_index) {
    for (int key_index = 0; key_index < max_mod_keys; ++key_index) {
      int key = mod_map->modifiermap[mod_index * max_mod_keys + key_index];
      int keysym = XkbKeycodeToKeysym(display, key, 0, 0);
      if (modifier == kAltKeyModifierMask)
        found = keysym == XK_Alt_L || keysym == XK_Alt_R;
      else if (modifier == kMetaKeyModifierMask)
        found = keysym == XK_Meta_L || keysym == XK_Meta_R;
      else if (modifier == kNumLockKeyModifierMask)
        found = keysym == XK_Num_Lock;
      if (found) {
        *x_modifier = 1 << mod_index;
        break;
      }
    }
    if (found)
      break;
  }
  XFreeModifiermap(mod_map);
  return found;
}

}  // namespace

bool ConvertKeyCodeToText(
    ui::KeyboardCode key_code, int modifiers, std::string* text,
    std::string* error_msg) {
  *error_msg = std::string();
  int x_key_code = KeyboardCodeToXKeyCode(key_code);
  if (x_key_code == -1) {
    *text = std::string();
    return true;
  }

  XEvent event;
  memset(&event, 0, sizeof(XEvent));
  XKeyEvent* key_event = &event.xkey;
  XDisplay* display = gfx::GetXDisplay();
  if (!display) {
    *error_msg =
        "an X display is required for keycode conversions, consider using Xvfb";
    *text = std::string();
    return false;
  }
  key_event->display = display;
  key_event->keycode = x_key_code;
  if (modifiers & kShiftKeyModifierMask)
    key_event->state |= ShiftMask;
  if (modifiers & kControlKeyModifierMask)
    key_event->state |= ControlMask;

  // Make a best attempt for non-standard modifiers.
  int x_modifier;
  if (modifiers & kAltKeyModifierMask &&
      GetXModifierMask(display, kAltKeyModifierMask, &x_modifier)) {
    key_event->state |= x_modifier;
  }
  if (modifiers & kMetaKeyModifierMask &&
      GetXModifierMask(display, kMetaKeyModifierMask, &x_modifier)) {
    key_event->state |= x_modifier;
  }
  if (modifiers & kNumLockKeyModifierMask &&
      GetXModifierMask(display, kNumLockKeyModifierMask, &x_modifier)) {
    key_event->state |= x_modifier;
  }
  key_event->type = KeyPress;
  uint16 character = ui::GetCharacterFromXEvent(&event);

  if (!character)
    *text = std::string();
  else
    *text = UTF16ToUTF8(string16(1, character));
  return true;
}

bool ConvertCharToKeyCode(
    char16 key,
    ui::KeyboardCode* key_code,
    int* necessary_modifiers,
    std::string* error_msg) {
  std::string key_string(UTF16ToUTF8(string16(1, key)));
  bool found = false;
  ui::KeyboardCode test_code;
  int test_modifiers;
  *error_msg = std::string();
  std::string conv_string;
  for (size_t i = 0; i < arraysize(kKeyCodeToXKeyCode); ++i) {
    test_code = kKeyCodeToXKeyCode[i].key_code;
    // Skip the numpad keys.
    if (test_code >= ui::VKEY_NUMPAD0 && test_code <= ui::VKEY_DIVIDE)
      continue;
    test_modifiers = 0;
    if (!ConvertKeyCodeToText(
        test_code, test_modifiers, &conv_string, error_msg))
      return false;
    if (conv_string == key_string) {
      found = true;
      break;
    }
    test_modifiers = kShiftKeyModifierMask;
    if (!ConvertKeyCodeToText(
        test_code, test_modifiers, &conv_string, error_msg))
      return false;
    if (conv_string == key_string) {
      found = true;
      break;
    }
  }
  if (found) {
    *key_code = test_code;
    *necessary_modifiers = test_modifiers;
  }
  return found;
}
