// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_autofill_client_desktop.h"

#include "xwalk/runtime/browser/ui/desktop/xwalk_autofill_popup_controller.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(xwalk::XWalkAutofillClientDesktop);

namespace xwalk {

XWalkAutofillClientDesktop::XWalkAutofillClientDesktop(
    content::WebContents* contents)
  : XWalkAutofillClient(contents) {
}

XWalkAutofillClientDesktop::~XWalkAutofillClientDesktop() {
  HideAutofillPopup();
}

void XWalkAutofillClientDesktop::HideAutofillPopupImpl() {
  if (popup_controller_)
    popup_controller_->Hide();
}

void XWalkAutofillClientDesktop::ShowAutofillPopupImpl(
    const gfx::RectF& element_bounds,
    base::i18n::TextDirection text_direction,
    const std::vector<autofill::Suggestion>& suggestions) {

  popup_controller_ =
      XWalkAutofillPopupController::GetOrCreate(popup_controller_,
          delegate_,
          web_contents_,
          web_contents_->GetNativeView(),
          element_bounds,
          text_direction);

  popup_controller_->Show(suggestions);
}

}  // namespace xwalk
