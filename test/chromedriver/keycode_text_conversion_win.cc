// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/keycode_text_conversion.h"

#include <windows.h>

#include <cctype>

#include "base/strings/utf_string_conversions.h"
#include "xwalk/test/chromedriver/chrome/ui_events.h"

bool ConvertKeyCodeToText(
    ui::KeyboardCode key_code, int modifiers, std::string* text,
    std::string* error_msg) {
  UINT scan_code = ::MapVirtualKeyW(key_code, MAPVK_VK_TO_VSC);
  BYTE keyboard_state[256];
  memset(keyboard_state, 0, 256);
  *error_msg = std::string();
  if (modifiers & kShiftKeyModifierMask)
    keyboard_state[VK_SHIFT] |= 0x80;
  if (modifiers & kControlKeyModifierMask)
    keyboard_state[VK_CONTROL] |= 0x80;
  if (modifiers & kAltKeyModifierMask)
    keyboard_state[VK_MENU] |= 0x80;
  wchar_t chars[5];
  int code = ::ToUnicode(key_code, scan_code, keyboard_state, chars, 4, 0);
  // |ToUnicode| converts some non-text key codes like F1 to various ASCII
  // control chars. Filter those out.
  if (code <= 0 || (code == 1 && std::iscntrl(chars[0])))
    *text = std::string();
  else
    WideToUTF8(chars, code, text);
  return true;
}

bool ConvertCharToKeyCode(
    char16 key, ui::KeyboardCode* key_code, int *necessary_modifiers,
    std::string* error_msg) {
  short vkey_and_modifiers = ::VkKeyScanW(key);
  bool translated = vkey_and_modifiers != -1 &&
                    LOBYTE(vkey_and_modifiers) != 0xFF &&
                    HIBYTE(vkey_and_modifiers) != 0xFF;
  *error_msg = std::string();
  if (translated) {
    *key_code = static_cast<ui::KeyboardCode>(LOBYTE(vkey_and_modifiers));
    int win_modifiers = HIBYTE(vkey_and_modifiers);
    int modifiers = 0;
    if (win_modifiers & 0x01)
      modifiers |= kShiftKeyModifierMask;
    if (win_modifiers & 0x02)
      modifiers |= kControlKeyModifierMask;
    if (win_modifiers & 0x04)
      modifiers |= kAltKeyModifierMask;
    // Ignore bit 0x08: It is for Hankaku key.
    *necessary_modifiers = modifiers;
  }
  return translated;
}
