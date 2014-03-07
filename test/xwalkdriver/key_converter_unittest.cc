// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <list>
#include <string>

#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/xwalk/ui_events.h"
#include "xwalk/test/xwalkdriver/key_converter.h"
#include "xwalk/test/xwalkdriver/test_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

void CheckEvents(const string16& keys,
                 KeyEvent expected_events[],
                 bool release_modifiers,
                 size_t expected_size,
                 int expected_modifiers) {
  int modifiers = 0;
  std::list<KeyEvent> events;
  EXPECT_EQ(kOk, ConvertKeysToKeyEvents(keys, release_modifiers,
                                        &modifiers, &events).code());
  EXPECT_EQ(expected_size, events.size());
  size_t i = 0;
  std::list<KeyEvent>::const_iterator it = events.begin();
  while (i < expected_size && it != events.end()) {
    EXPECT_EQ(expected_events[i].type, it->type);
    EXPECT_EQ(expected_events[i].modifiers, it->modifiers);
    EXPECT_EQ(expected_events[i].modified_text, it->modified_text);
    EXPECT_EQ(expected_events[i].unmodified_text, it->unmodified_text);
    EXPECT_EQ(expected_events[i].key_code, it->key_code);

    ++i;
    ++it;
  }
  EXPECT_EQ(expected_modifiers, modifiers);
}

void CheckEventsReleaseModifiers(const string16& keys,
                                 KeyEvent expected_events[],
                                 size_t expected_size) {
  CheckEvents(keys, expected_events, true /* release_modifier */,
      expected_size, 0 /* expected_modifiers */);
}

void CheckEventsReleaseModifiers(const std::string& keys,
                                 KeyEvent expected_events[],
                                 size_t expected_size) {
  CheckEventsReleaseModifiers(UTF8ToUTF16(keys),
      expected_events, expected_size);
}

void CheckNonShiftChar(ui::KeyboardCode key_code, char character) {
  int modifiers = 0;
  std::string char_string;
  char_string.push_back(character);
  std::list<KeyEvent> events;
  EXPECT_EQ(kOk, ConvertKeysToKeyEvents(ASCIIToUTF16(char_string),
                                        true /* release_modifiers*/,
                                        &modifiers, &events).code());
  ASSERT_EQ(3u, events.size()) << "Char: " << character;
  std::list<KeyEvent>::const_iterator it = events.begin();
  EXPECT_EQ(key_code, it->key_code) << "Char: " << character;
  ++it;  // Move to the second event.
  ASSERT_EQ(1u, it->modified_text.length()) << "Char: " << character;
  ASSERT_EQ(1u, it->unmodified_text.length()) << "Char: " << character;
  EXPECT_EQ(character, it->modified_text[0]) << "Char: " << character;
  EXPECT_EQ(character, it->unmodified_text[0]) << "Char: " << character;
  ++it;  // Move to the third event.
  EXPECT_EQ(key_code, it->key_code) << "Char: " << character;
}

void CheckShiftChar(ui::KeyboardCode key_code, char character, char lower) {
  int modifiers = 0;
  std::string char_string;
  char_string.push_back(character);
  std::list<KeyEvent> events;
  EXPECT_EQ(kOk, ConvertKeysToKeyEvents(ASCIIToUTF16(char_string),
                                        true /* release_modifiers*/,
                                        &modifiers, &events).code());
  ASSERT_EQ(5u, events.size()) << "Char: " << character;
  std::list<KeyEvent>::const_iterator it = events.begin();
  EXPECT_EQ(ui::VKEY_SHIFT, it->key_code) << "Char: " << character;
  ++it;  // Move to second event.
  EXPECT_EQ(key_code, it->key_code) << "Char: " << character;
  ++it;  // Move to third event.
  ASSERT_EQ(1u, it->modified_text.length()) << "Char: " << character;
  ASSERT_EQ(1u, it->unmodified_text.length()) << "Char: " << character;
  EXPECT_EQ(character, it->modified_text[0]) << "Char: " << character;
  EXPECT_EQ(lower, it->unmodified_text[0]) << "Char: " << character;
  ++it;  // Move to fourth event.
  EXPECT_EQ(key_code, it->key_code) << "Char: " << character;
  ++it;  // Move to fifth event.
  EXPECT_EQ(ui::VKEY_SHIFT, it->key_code) << "Char: " << character;
}

}  // namespace

TEST(KeyConverter, SingleChar) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_H, 0),
      CreateCharEvent("h", "h", 0),
      CreateKeyUpEvent(ui::VKEY_H, 0)};
  CheckEventsReleaseModifiers("h", event_array, arraysize(event_array));
}

TEST(KeyConverter, SingleNumber) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_1, 0),
      CreateCharEvent("1", "1", 0),
      CreateKeyUpEvent(ui::VKEY_1, 0)};
  CheckEventsReleaseModifiers("1", event_array, arraysize(event_array));
}

TEST(KeyConverter, MultipleChars) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_H, 0),
      CreateCharEvent("h", "h", 0),
      CreateKeyUpEvent(ui::VKEY_H, 0),
      CreateKeyDownEvent(ui::VKEY_E, 0),
      CreateCharEvent("e", "e", 0),
      CreateKeyUpEvent(ui::VKEY_E, 0),
      CreateKeyDownEvent(ui::VKEY_Y, 0),
      CreateCharEvent("y", "y", 0),
      CreateKeyUpEvent(ui::VKEY_Y, 0)};
  CheckEventsReleaseModifiers("hey", event_array, arraysize(event_array));
}

TEST(KeyConverter, WebDriverSpecialChar) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_SPACE, 0),
      CreateCharEvent(" ", " ", 0),
      CreateKeyUpEvent(ui::VKEY_SPACE, 0)};
  string16 keys;
  keys.push_back(static_cast<char16>(0xE00DU));
  CheckEventsReleaseModifiers(keys, event_array, arraysize(event_array));
}

TEST(KeyConverter, WebDriverSpecialNonCharKey) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_F1, 0),
      CreateKeyUpEvent(ui::VKEY_F1, 0)};
  string16 keys;
  keys.push_back(static_cast<char16>(0xE031U));
  CheckEventsReleaseModifiers(keys, event_array, arraysize(event_array));
}

TEST(KeyConverter, FrenchKeyOnEnglishLayout) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_UNKNOWN, 0),
      CreateCharEvent(WideToUTF8(L"\u00E9"), WideToUTF8(L"\u00E9"), 0),
      CreateKeyUpEvent(ui::VKEY_UNKNOWN, 0)};
  CheckEventsReleaseModifiers(WideToUTF16(L"\u00E9"),
      event_array, arraysize(event_array));
}

#if defined(OS_WIN)
TEST(KeyConverter, NeedsCtrlAndAlt) {
  RestoreKeyboardLayoutOnDestruct restore;
  int ctrl_and_alt = kControlKeyModifierMask | kAltKeyModifierMask;
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_CONTROL, 0),
      CreateKeyDownEvent(ui::VKEY_MENU, 0),
      CreateKeyDownEvent(ui::VKEY_Q, ctrl_and_alt),
      CreateCharEvent("q", "@", ctrl_and_alt),
      CreateKeyUpEvent(ui::VKEY_Q, ctrl_and_alt),
      CreateKeyUpEvent(ui::VKEY_MENU, 0),
      CreateKeyUpEvent(ui::VKEY_CONTROL, 0)};
  ASSERT_TRUE(SwitchKeyboardLayout("00000407"));
  CheckEventsReleaseModifiers("@", event_array, arraysize(event_array));
}
#endif

TEST(KeyConverter, UppercaseCharDoesShift) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_SHIFT, 0),
      CreateKeyDownEvent(ui::VKEY_A, kShiftKeyModifierMask),
      CreateCharEvent("a", "A", kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_A, kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_SHIFT, 0)};
  CheckEventsReleaseModifiers("A", event_array, arraysize(event_array));
}

TEST(KeyConverter, UppercaseSymbolCharDoesShift) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_SHIFT, 0),
      CreateKeyDownEvent(ui::VKEY_1, kShiftKeyModifierMask),
      CreateCharEvent("1", "!", kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_1, kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_SHIFT, 0)};
  CheckEventsReleaseModifiers("!", event_array, arraysize(event_array));
}

TEST(KeyConverter, UppercaseCharUsesShiftOnlyIfNecessary) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_SHIFT, kShiftKeyModifierMask),
      CreateKeyDownEvent(ui::VKEY_A, kShiftKeyModifierMask),
      CreateCharEvent("a", "A", kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_A, kShiftKeyModifierMask),
      CreateKeyDownEvent(ui::VKEY_B, kShiftKeyModifierMask),
      CreateCharEvent("b", "B", kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_B, kShiftKeyModifierMask),
      CreateKeyDownEvent(ui::VKEY_C, kShiftKeyModifierMask),
      CreateCharEvent("c", "C", kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_C, kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_SHIFT, 0)};
  string16 keys;
  keys.push_back(static_cast<char16>(0xE008U));
  keys.append(UTF8ToUTF16("aBc"));
  CheckEventsReleaseModifiers(keys, event_array, arraysize(event_array));
}

TEST(KeyConverter, ToggleModifiers) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_SHIFT, kShiftKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_SHIFT, 0),
      CreateKeyDownEvent(ui::VKEY_CONTROL, kControlKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_CONTROL, 0),
      CreateKeyDownEvent(ui::VKEY_MENU, kAltKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_MENU, 0),
      CreateKeyDownEvent(ui::VKEY_COMMAND, kMetaKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_COMMAND, 0)};
  string16 keys;
  keys.push_back(static_cast<char16>(0xE008U));
  keys.push_back(static_cast<char16>(0xE008U));
  keys.push_back(static_cast<char16>(0xE009U));
  keys.push_back(static_cast<char16>(0xE009U));
  keys.push_back(static_cast<char16>(0xE00AU));
  keys.push_back(static_cast<char16>(0xE00AU));
  keys.push_back(static_cast<char16>(0xE03DU));
  keys.push_back(static_cast<char16>(0xE03DU));
  CheckEventsReleaseModifiers(keys, event_array, arraysize(event_array));
}

#if defined(OS_WIN)
// https://code.google.com/p/chromedriver/issues/detail?id=546
#define MAYBE_AllShorthandKeys DISABLED_AllShorthandKeys
#else
#define MAYBE_AllShorthandKeys AllShorthandKeys
#endif

TEST(KeyConverter, MAYBE_AllShorthandKeys) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_RETURN, 0),
      CreateCharEvent("\r", "\r", 0),
      CreateKeyUpEvent(ui::VKEY_RETURN, 0),
      CreateKeyDownEvent(ui::VKEY_RETURN, 0),
      CreateCharEvent("\r", "\r", 0),
      CreateKeyUpEvent(ui::VKEY_RETURN, 0),
      CreateKeyDownEvent(ui::VKEY_TAB, 0),
#if defined(USE_AURA) || defined(OS_LINUX)
      CreateCharEvent("\t", "\t", 0),
#endif
      CreateKeyUpEvent(ui::VKEY_TAB, 0),
      CreateKeyDownEvent(ui::VKEY_BACK, 0),
#if defined(USE_AURA) || defined(OS_LINUX)
      CreateCharEvent("\b", "\b", 0),
#endif
      CreateKeyUpEvent(ui::VKEY_BACK, 0),
      CreateKeyDownEvent(ui::VKEY_SPACE, 0),
      CreateCharEvent(" ", " ", 0),
      CreateKeyUpEvent(ui::VKEY_SPACE, 0)};
  CheckEventsReleaseModifiers("\n\r\n\t\b ",
      event_array,arraysize(event_array));  // NOLINT
}

#if defined(OS_LINUX)
// Fails on bots: crbug.com/174962
#define MAYBE_AllEnglishKeyboardSymbols DISABLED_AllEnglishKeyboardSymbols
#else
#define MAYBE_AllEnglishKeyboardSymbols AllEnglishKeyboardSymbols
#endif

TEST(KeyConverter, MAYBE_AllEnglishKeyboardSymbols) {
  string16 keys;
  const ui::KeyboardCode kSymbolKeyCodes[] = {
      ui::VKEY_OEM_3,
      ui::VKEY_OEM_MINUS,
      ui::VKEY_OEM_PLUS,
      ui::VKEY_OEM_4,
      ui::VKEY_OEM_6,
      ui::VKEY_OEM_5,
      ui::VKEY_OEM_1,
      ui::VKEY_OEM_7,
      ui::VKEY_OEM_COMMA,
      ui::VKEY_OEM_PERIOD,
      ui::VKEY_OEM_2};
  std::string kLowerSymbols = "`-=[]\\;',./";
  std::string kUpperSymbols = "~_+{}|:\"<>?";
  for (size_t i = 0; i < kLowerSymbols.length(); ++i)
    CheckNonShiftChar(kSymbolKeyCodes[i], kLowerSymbols[i]);
  for (size_t i = 0; i < kUpperSymbols.length(); ++i)
    CheckShiftChar(kSymbolKeyCodes[i], kUpperSymbols[i], kLowerSymbols[i]);
}

TEST(KeyConverter, AllEnglishKeyboardTextChars) {
  std::string kLowerChars = "0123456789abcdefghijklmnopqrstuvwxyz";
  std::string kUpperChars = ")!@#$%^&*(ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for (size_t i = 0; i < kLowerChars.length(); ++i) {
    int offset = 0;
    if (i < 10)
      offset = ui::VKEY_0;
    else
      offset = ui::VKEY_0 + 7;
    ui::KeyboardCode expected_code = static_cast<ui::KeyboardCode>(offset + i);
    CheckNonShiftChar(expected_code, kLowerChars[i]);
  }
  for (size_t i = 0; i < kUpperChars.length(); ++i) {
    int offset = 0;
    if (i < 10)
      offset = ui::VKEY_0;
    else
      offset = ui::VKEY_0 + 7;
    ui::KeyboardCode expected_code = static_cast<ui::KeyboardCode>(offset + i);
    CheckShiftChar(expected_code, kUpperChars[i], kLowerChars[i]);
  }
}

#if defined(OS_LINUX) || defined(OS_WIN)
// https://code.google.com/p/chromedriver/issues/detail?id=240
// https://code.google.com/p/chromedriver/issues/detail?id=546
#define MAYBE_AllSpecialWebDriverKeysOnEnglishKeyboard \
    DISABLED_AllSpecialWebDriverKeysOnEnglishKeyboard
#else
#define MAYBE_AllSpecialWebDriverKeysOnEnglishKeyboard \
    AllSpecialWebDriverKeysOnEnglishKeyboard
#endif

TEST(KeyConverter, MAYBE_AllSpecialWebDriverKeysOnEnglishKeyboard) {
  const char kTextForKeys[] = {
#if defined(USE_AURA) || defined(OS_LINUX)
      0, 0, 0, '\b', '\t', 0, '\r', '\r', 0, 0, 0, 0, 0x1B,
      ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F, ';', '=',
#else
      0, 0, 0, 0, 0, 0, '\r', '\r', 0, 0, 0, 0, 0,
      ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ';', '=',
#endif
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      '*', '+', ',', '-', '.', '/'};
  for (size_t i = 0; i <= 0x3D; ++i) {
    if (i > 0x29 && i < 0x31)
      continue;
    string16 keys;
    int modifiers = 0;
    keys.push_back(0xE000U + i);
    std::list<KeyEvent> events;
    if (i == 1) {
      EXPECT_NE(kOk, ConvertKeysToKeyEvents(keys,
                                            true /* release_modifiers*/,
                                            &modifiers, &events).code())
          << "Index: " << i;
      EXPECT_EQ(0u, events.size()) << "Index: " << i;
    } else {
      EXPECT_EQ(kOk, ConvertKeysToKeyEvents(keys,
                                            true /* release_modifiers */,
                                            &modifiers, &events).code())
          << "Index: " << i;
      if (i == 0) {
        EXPECT_EQ(0u, events.size()) << "Index: " << i;
      } else if (i >= arraysize(kTextForKeys) || kTextForKeys[i] == 0) {
        EXPECT_EQ(2u, events.size()) << "Index: " << i;
      } else {
        ASSERT_EQ(3u, events.size()) << "Index: " << i;
        std::list<KeyEvent>::const_iterator it = events.begin();
        ++it;  // Move to the second event.
        ASSERT_EQ(1u, it->unmodified_text.length()) << "Index: " << i;
        EXPECT_EQ(kTextForKeys[i], it->unmodified_text[0])
            << "Index: " << i;
      }
    }
  }
}

TEST(KeyConverter, ModifiersState) {
  int shift_key_modifier = kShiftKeyModifierMask;
  int control_key_modifier = shift_key_modifier | kControlKeyModifierMask;
  int alt_key_modifier = control_key_modifier | kAltKeyModifierMask;
  int meta_key_modifier = alt_key_modifier | kMetaKeyModifierMask;
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_SHIFT, shift_key_modifier),
      CreateKeyDownEvent(ui::VKEY_CONTROL, control_key_modifier),
      CreateKeyDownEvent(ui::VKEY_MENU, alt_key_modifier),
      CreateKeyDownEvent(ui::VKEY_COMMAND, meta_key_modifier)};
  string16 keys;
  keys.push_back(static_cast<char16>(0xE008U));
  keys.push_back(static_cast<char16>(0xE009U));
  keys.push_back(static_cast<char16>(0xE00AU));
  keys.push_back(static_cast<char16>(0xE03DU));

  CheckEvents(keys, event_array, false /* release_modifiers */,
      arraysize(event_array), meta_key_modifier);
}

TEST(KeyConverter, ReleaseModifiers) {
  KeyEvent event_array[] = {
      CreateKeyDownEvent(ui::VKEY_SHIFT, kShiftKeyModifierMask),
      CreateKeyDownEvent(ui::VKEY_CONTROL,
          kShiftKeyModifierMask | kControlKeyModifierMask),
      CreateKeyUpEvent(ui::VKEY_SHIFT, 0),
      CreateKeyUpEvent(ui::VKEY_CONTROL, 0)};
  string16 keys;
  keys.push_back(static_cast<char16>(0xE008U));
  keys.push_back(static_cast<char16>(0xE009U));

  CheckEvents(keys, event_array, true /* release_modifiers */,
      arraysize(event_array), 0);
}
