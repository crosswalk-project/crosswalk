// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_autofill_client_desktop.h"

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
  NOTIMPLEMENTED();
}

void XWalkAutofillClientDesktop::ShowAutofillPopupImpl(
    const gfx::RectF& element_bounds,
    bool is_rtl,
    const std::vector<autofill::Suggestion>& suggestions) {
  NOTIMPLEMENTED();
}

}  // namespace xwalk
