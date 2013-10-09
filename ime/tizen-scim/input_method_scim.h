// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_IME_INPUT_METHOD_SCIM_H_
#define UI_BASE_IME_INPUT_METHOD_SCIM_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/observer_list.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/ime/input_method.h"
#include "ui/base/ime/input_method_observer.h"
#include "ui/base/ui_export.h"

namespace ui {

class InputMethodObserver;
class KeyEvent;
class TextInputClient;
class SCIMBridge;

// SCIM InputMethod implementation for minimum input support.
class UI_EXPORT InputMethodSCIM : NON_EXPORTED_BASE(public InputMethod) {
 public:
  explicit InputMethodSCIM(internal::InputMethodDelegate* delegate);
  virtual ~InputMethodSCIM();

  // Overriden from InputMethod.
  virtual void SetDelegate(internal::InputMethodDelegate* delegate) OVERRIDE;
  virtual void Init(bool focused) OVERRIDE;
  virtual void OnFocus() OVERRIDE;
  virtual void OnBlur() OVERRIDE;
  virtual bool OnUntranslatedIMEMessage(const base::NativeEvent& event,
                                        NativeEventResult* result) OVERRIDE;
  virtual void SetFocusedTextInputClient(TextInputClient* client) OVERRIDE;
  virtual TextInputClient* GetTextInputClient() const OVERRIDE;
  virtual bool DispatchKeyEvent(const base::NativeEvent& native_event) OVERRIDE;
  virtual bool DispatchFabricatedKeyEvent(const ui::KeyEvent& event) OVERRIDE;
  virtual void OnTextInputTypeChanged(const TextInputClient* client) OVERRIDE;
  virtual void OnCaretBoundsChanged(const TextInputClient* client) OVERRIDE;
  virtual void CancelComposition(const TextInputClient* client) OVERRIDE;
  virtual void OnInputLocaleChanged() OVERRIDE;
  virtual std::string GetInputLocale() OVERRIDE;
  virtual base::i18n::TextDirection GetInputTextDirection() OVERRIDE;
  virtual bool IsActive() OVERRIDE;
  virtual ui::TextInputType GetTextInputType() const OVERRIDE;
  virtual bool CanComposeInline() const OVERRIDE;
  virtual bool IsCandidatePopupOpen() const OVERRIDE;
  virtual void AddObserver(InputMethodObserver* observer) OVERRIDE;
  virtual void RemoveObserver(InputMethodObserver* observer) OVERRIDE;

  virtual void SetStickyFocusedTextInputClient(ui::TextInputClient*) OVERRIDE;
  virtual void DetachTextInputClient(ui::TextInputClient*) OVERRIDE;
  virtual ui::TextInputMode GetTextInputMode() const OVERRIDE;


 private:
  internal::InputMethodDelegate* delegate_;
  TextInputClient* text_input_client_;
  ObserverList<InputMethodObserver> observers_;
  scoped_ptr<SCIMBridge> scim_bridge_;

  DISALLOW_COPY_AND_ASSIGN(InputMethodSCIM);
};

}  // namespace ui

#endif  // UI_BASE_IME_INPUT_METHOD_SCIM_H_
