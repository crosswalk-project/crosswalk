// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_CLIENT_DESKTOP_H_
#define XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_CLIENT_DESKTOP_H_

#include <vector>

#include "xwalk/runtime/browser/xwalk_autofill_client.h"

namespace gfx {
class RectF;
}

namespace xwalk {

class XWalkAutofillPopupController;

class XWalkAutofillClientDesktop :
    public XWalkAutofillClient,
    public content::WebContentsUserData<XWalkAutofillClientDesktop> {
 private:
  explicit XWalkAutofillClientDesktop(content::WebContents* web_contents);
  ~XWalkAutofillClientDesktop() override;
  friend class content::WebContentsUserData<XWalkAutofillClientDesktop>;

  void HideAutofillPopupImpl() override;

  void ShowAutofillPopupImpl(
      const gfx::RectF& element_bounds,
      base::i18n::TextDirection text_direction,
      const std::vector<autofill::Suggestion>& suggestions) override;

  base::WeakPtr<XWalkAutofillPopupController> popup_controller_;

  DISALLOW_COPY_AND_ASSIGN(XWalkAutofillClientDesktop);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_CLIENT_DESKTOP_H_
