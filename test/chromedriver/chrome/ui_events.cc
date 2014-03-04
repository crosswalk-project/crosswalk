// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome/ui_events.h"

MouseEvent::MouseEvent(MouseEventType type,
                       MouseButton button,
                       int x,
                       int y,
                       int modifiers,
                       int click_count)
    : type(type),
      button(button),
      x(x),
      y(y),
      modifiers(modifiers),
      click_count(click_count) {}

MouseEvent::~MouseEvent() {}

TouchEvent::TouchEvent(TouchEventType type,
                       int x,
                       int y)
    : type(type),
      x(x),
      y(y) {}

TouchEvent::~TouchEvent() {}

KeyEvent::KeyEvent(KeyEventType type,
                   int modifiers,
                   const std::string& modified_text,
                   const std::string& unmodified_text,
                   ui::KeyboardCode key_code)
    : type(type),
      modifiers(modifiers),
      modified_text(modified_text),
      unmodified_text(unmodified_text),
      key_code(key_code) {}

KeyEvent::~KeyEvent() {}
