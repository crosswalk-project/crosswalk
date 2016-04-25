// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_WIN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_WIN_H_

#include <string>
#include "xwalk/runtime/browser/xwalk_presentation_service_delegate.h"

namespace xwalk {

class XWalkPresentationServiceDelegateWin
    : public XWalkPresentationServiceDelegate,
      public content::WebContentsUserData<XWalkPresentationServiceDelegateWin>,
      public base::SupportsWeakPtr<XWalkPresentationServiceDelegateWin> {
 public:
  static content::PresentationServiceDelegate* GetOrCreateForWebContents(
      content::WebContents* web_contents);

  ~XWalkPresentationServiceDelegateWin() override;

 private:
  explicit XWalkPresentationServiceDelegateWin(
      content::WebContents* web_contents);

  friend class content::WebContentsUserData<
      XWalkPresentationServiceDelegateWin>;

 public:
  void StartSession(
      int render_process_id,
      int render_frame_id,
      const std::string& presentation_url,
      const content::PresentationSessionStartedCallback& success_cb,
      const content::PresentationSessionErrorCallback& error_cb) override;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_WIN_H_
