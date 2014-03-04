// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_UI_EVENTS_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_UI_EVENTS_H_

#include <string>

#include "ui/events/keycodes/keyboard_codes.h"

// Specifies the type of the mouse event.
enum MouseEventType {
  kPressedMouseEventType = 0,
  kReleasedMouseEventType,
  kMovedMouseEventType
};

// Specifies the mouse buttons.
enum MouseButton {
  kLeftMouseButton = 0,
  kMiddleMouseButton,
  kRightMouseButton,
  kNoneMouseButton
};

struct MouseEvent {
  MouseEvent(MouseEventType type,
             MouseButton button,
             int x,
             int y,
             int modifiers,
             int click_count);
  ~MouseEvent();

  MouseEventType type;
  MouseButton button;
  int x;
  int y;
  int modifiers;
  // |click_count| should not be negative.
  int click_count;
};

// Specifies the type of the touch event.
enum TouchEventType {
  kTouchStart = 0,
  kTouchEnd,
  kTouchMove,
};

struct TouchEvent {
  TouchEvent(TouchEventType type,
             int x,
             int y);
  ~TouchEvent();

  TouchEventType type;
  int x;
  int y;
};

// Specifies the type of the keyboard event.
enum KeyEventType {
  kKeyDownEventType = 0,
  kKeyUpEventType,
  kRawKeyDownEventType,
  kCharEventType
};

// Specifies modifier keys as stated in
// third_party/WebKit/Source/WebCore/inspector/Inspector.json.
// Notice: |kNumLockKeyModifierMask| is for usage in the key_converter.cc
//         and keycode_text_conversion_x.cc only, not for inspector.
enum KeyModifierMask {
  kAltKeyModifierMask = 1 << 0,
  kControlKeyModifierMask = 1 << 1,
  kMetaKeyModifierMask = 1 << 2,
  kShiftKeyModifierMask = 1 << 3,
  kNumLockKeyModifierMask = 1 << 4
};

struct KeyEvent {
  KeyEvent(KeyEventType type,
           int modifiers,
           const std::string& modified_text,
           const std::string& unmodified_text,
           ui::KeyboardCode key_code);
  ~KeyEvent();

  KeyEventType type;
  int modifiers;
  std::string modified_text;
  std::string unmodified_text;
  ui::KeyboardCode key_code;
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_UI_EVENTS_H_
