// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_ANDROID_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_ANDROID_H_

#include <string>
#include "xwalk/runtime/browser/xwalk_presentation_service_delegate.h"

namespace xwalk {

class XWalkPresentationServiceDelegateAndroid
    : public XWalkPresentationServiceDelegate,
      public content::WebContentsUserData<
          XWalkPresentationServiceDelegateAndroid>,
      public base::SupportsWeakPtr<XWalkPresentationServiceDelegateAndroid> {
 public:
  static content::PresentationServiceDelegate* GetOrCreateForWebContents(
      content::WebContents* web_contents);

  ~XWalkPresentationServiceDelegateAndroid() override;

 private:
  explicit XWalkPresentationServiceDelegateAndroid(
      content::WebContents* web_contents);

  friend class content::WebContentsUserData<
      XWalkPresentationServiceDelegateAndroid>;

 public:
  void StartSession(
      int render_process_id,
      int render_frame_id,
      const std::string& presentation_url,
      const content::PresentationSessionStartedCallback& success_cb,
      const content::PresentationSessionErrorCallback& error_cb) override;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_ANDROID_H_
