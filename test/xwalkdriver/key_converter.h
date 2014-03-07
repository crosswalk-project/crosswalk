// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_KEY_CONVERTER_H_
#define XWALK_TEST_XWALKDRIVER_KEY_CONVERTER_H_

#include <list>
#include <string>

#include "base/strings/string16.h"
#include "ui/events/keycodes/keyboard_codes.h"

struct KeyEvent;
class Status;

// Convenience functions for creating |KeyEvent|s. Used by unittests.
KeyEvent CreateKeyDownEvent(ui::KeyboardCode key_code, int modifiers);
KeyEvent CreateKeyUpEvent(ui::KeyboardCode key_code, int modifiers);
KeyEvent CreateCharEvent(const std::string& unmodified_text,
                         const std::string& modified_text,
                         int modifiers);

// Converts keys into appropriate |KeyEvent|s. This will do a best effort
// conversion. However, if the input is invalid it will return a status with
// an error message. If |release_modifiers| is true, all modifiers would be
// depressed. |modifiers| acts both an input and an output, however, only when
// the conversion process is successful will |modifiers| be changed.
Status ConvertKeysToKeyEvents(const string16& keys,
                              bool release_modifiers,
                              int* modifiers,
                              std::list<KeyEvent>* key_events);

#endif  // XWALK_TEST_XWALKDRIVER_KEY_CONVERTER_H_
