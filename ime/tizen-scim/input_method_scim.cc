// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/ime/tizen-scim/input_method_scim.h"

#include <X11/X.h>

#include "base/logging.h"
#include "ui/aura/root_window.h"
#include "ui/base/ime/input_method_delegate.h"
#include "ui/base/ime/text_input_client.h"
#include "ui/events/event_utils.h"
#include "ui/events/keycodes/keyboard_code_conversion.h"
#include "ui/events/keycodes/keyboard_code_conversion_x.h"
#include "xwalk/ime/tizen-scim/scim_bridge.h"

// Xlib.h must include after "root_window.h" otherwise compiler complains.
#include <X11/Xlib.h> // NOLINT

namespace {

uint32 EventFlagsFromXFlags(unsigned int flags) {
  return (flags & LockMask ? ui::EF_CAPS_LOCK_DOWN : 0U)
         | (flags & ControlMask ? ui::EF_CONTROL_DOWN : 0U)
         | (flags & ShiftMask ? ui::EF_SHIFT_DOWN : 0U)
         | (flags & Mod1Mask ? ui::EF_ALT_DOWN : 0U);
}

}  // namespace

namespace ui {

InputMethodSCIM::InputMethodSCIM(internal::InputMethodDelegate* delegate)
    : delegate_(NULL),
      text_input_client_(NULL),
      scim_bridge_(new SCIMBridge()) {
  SetDelegate(delegate);
}

InputMethodSCIM::~InputMethodSCIM() {
}

void InputMethodSCIM::SetDelegate(internal::InputMethodDelegate* delegate) {
  delegate_ = delegate;
}

void InputMethodSCIM::SetFocusedTextInputClient(TextInputClient* client) {
  if (client && client->GetAttachedWindow()) {
    aura::Window* rw = client->GetAttachedWindow()->GetRootWindow();
      if (rw) {
        scim_bridge_->SetFocusedWindow(
            rw->GetDispatcher()->GetAcceleratedWidget());
        scim_bridge_->Init();
      }
  }

  text_input_client_ = client;
  FOR_EACH_OBSERVER(InputMethodObserver, observers_,
                    OnTextInputStateChanged(client));
}

TextInputClient* InputMethodSCIM::GetTextInputClient() const {
  return text_input_client_;
}

bool InputMethodSCIM::DispatchKeyEvent(const base::NativeEvent& native_event) {
  bool handled = false;
  DCHECK(native_event);
  if (native_event->type == KeyRelease) {
    // On key release, just dispatch it.
    handled = delegate_->DispatchKeyEventPostIME(native_event);
  } else {
    const uint32 state = EventFlagsFromXFlags(native_event->xkey.state);
    // Send a RawKeyDown event first.
    handled = delegate_->DispatchKeyEventPostIME(native_event);
    if (text_input_client_) {
      // then send a Char event via ui::TextInputClient.
      const KeyboardCode key_code = ui::KeyboardCodeFromNative(native_event);
      uint16 ch = 0;
      if (!(state & ui::EF_CONTROL_DOWN))
        ch = ui::GetCharacterFromXEvent(native_event);
      if (!ch)
        ch = ui::GetCharacterFromKeyCode(key_code, state);
      if (ch)
        text_input_client_->InsertChar(ch, state);
    }
  }
  return handled;
}

bool InputMethodSCIM::DispatchFabricatedKeyEvent(const ui::KeyEvent& event) {
  bool handled = delegate_->DispatchFabricatedKeyEventPostIME(
      event.type(), event.key_code(), event.flags());
  if (event.type() == ET_KEY_PRESSED && text_input_client_) {
    if (uint16 ch = event.GetCharacter())
      text_input_client_->InsertChar(ch, event.flags());
  }
  return handled;
}

void InputMethodSCIM::Init(bool focused) {}
void InputMethodSCIM::OnFocus() {}
void InputMethodSCIM::OnBlur() {}
bool InputMethodSCIM::OnUntranslatedIMEMessage(const base::NativeEvent& event,
                                               NativeEventResult* result) {
  return false;
}

void InputMethodSCIM::OnTextInputTypeChanged(const TextInputClient* client) {
  if (client) {
    scim_bridge_->TextInputChanged(client->GetTextInputType());
  }

  FOR_EACH_OBSERVER(InputMethodObserver, observers_,
                    OnTextInputStateChanged(client));
}

void InputMethodSCIM::OnCaretBoundsChanged(const TextInputClient* client) {}
void InputMethodSCIM::CancelComposition(const TextInputClient* client) {}
void InputMethodSCIM::OnInputLocaleChanged() {}

std::string InputMethodSCIM::GetInputLocale() {
  // TODO(alexshalamov): Fetch locale from SCIM.
  return "";
}

base::i18n::TextDirection InputMethodSCIM::GetInputTextDirection() {
  // TODO(alexshalamov): Fetch input text direction from SCIM.
  return base::i18n::UNKNOWN_DIRECTION;
}

bool InputMethodSCIM::IsActive() {
  return true;
}

bool InputMethodSCIM::IsCandidatePopupOpen() const {
  return false;
}

ui::TextInputType InputMethodSCIM::GetTextInputType() const {
  return ui::TEXT_INPUT_TYPE_NONE;
}

bool InputMethodSCIM::CanComposeInline() const {
  return true;
}

void InputMethodSCIM::AddObserver(InputMethodObserver* observer) {
  observers_.AddObserver(observer);
}

void InputMethodSCIM::RemoveObserver(InputMethodObserver* observer) {
  observers_.RemoveObserver(observer);
}

void InputMethodSCIM::SetStickyFocusedTextInputClient(ui::TextInputClient*) {
  // TODO(shalamov): Not implemented.
}

void InputMethodSCIM::DetachTextInputClient(ui::TextInputClient*) {
  // TODO(shalamov): Not implemented.
}

ui::TextInputMode InputMethodSCIM::GetTextInputMode() const {
  return ui::TEXT_INPUT_MODE_DEFAULT;
}

}  // namespace ui
