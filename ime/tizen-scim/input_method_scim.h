// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_IME_TIZEN_SCIM_INPUT_METHOD_SCIM_H_
#define XWALK_IME_TIZEN_SCIM_INPUT_METHOD_SCIM_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/observer_list.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/ime/input_method.h"
#include "ui/base/ime/input_method_observer.h"
#include "ui/base/ui_base_export.h"

namespace ui {

class InputMethodObserver;
class KeyEvent;
class TextInputClient;
class SCIMBridge;

// SCIM InputMethod implementation for minimum input support.
class UI_BASE_EXPORT InputMethodSCIM : NON_EXPORTED_BASE(public InputMethod) {
 public:
  explicit InputMethodSCIM(internal::InputMethodDelegate* delegate);
  virtual ~InputMethodSCIM();

  // Overriden from InputMethod.
  void SetDelegate(internal::InputMethodDelegate* delegate) override;
  void Init(bool focused) override;
  void OnFocus() override;
  void OnBlur() override;
  bool OnUntranslatedIMEMessage(const base::NativeEvent& event,
                                NativeEventResult* result) override;
  void SetFocusedTextInputClient(TextInputClient* client) override;
  TextInputClient* GetTextInputClient() const override;
  bool DispatchKeyEvent(const ui::KeyEvent& event) override;
  bool DispatchFabricatedKeyEvent(const ui::KeyEvent& event) override;
  void OnTextInputTypeChanged(const TextInputClient* client) override;
  void OnCaretBoundsChanged(const TextInputClient* client) override;
  void CancelComposition(const TextInputClient* client) override;
  void OnInputLocaleChanged() override;
  std::string GetInputLocale() override;
  bool IsActive() override;
  ui::TextInputType GetTextInputType() const override;
  bool CanComposeInline() const override;
  bool IsCandidatePopupOpen() const override;
  void ShowImeIfNeeded() override;
  void AddObserver(InputMethodObserver* observer) override;
  void RemoveObserver(InputMethodObserver* observer) override;

  void SetStickyFocusedTextInputClient(ui::TextInputClient*) override;
  void DetachTextInputClient(ui::TextInputClient*) override;
  ui::TextInputMode GetTextInputMode() const override;


 private:
  internal::InputMethodDelegate* delegate_;
  TextInputClient* text_input_client_;
  ObserverList<InputMethodObserver> observers_;
  scoped_ptr<SCIMBridge> scim_bridge_;

  DISALLOW_COPY_AND_ASSIGN(InputMethodSCIM);
};

}  // namespace ui

#endif  // XWALK_IME_TIZEN_SCIM_INPUT_METHOD_SCIM_H_
