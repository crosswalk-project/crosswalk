// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_XWALK_HTTP_AUTH_HANDLER_BASE_H_
#define ANDROID_XWALK_HTTP_AUTH_HANDLER_BASE_H_

namespace content {
class WebContents;
};

namespace net {
class AuthChallengeInfo;
};

namespace xwalk {

class XWalkLoginDelegate;

// browser/ layer interface for XWalkHttpAuthHandler (which is implemented in the
// native/ layer as a native version of the Java class of the same name). This
// allows the browser/ layer to be unaware of JNI/Java shenanigans.
class XWalkHttpAuthHandlerBase {
 public:
  static XWalkHttpAuthHandlerBase* Create(XWalkLoginDelegate* login_delegate,
                                       net::AuthChallengeInfo* auth_info,
                                       bool first_auth_attempt);
  virtual ~XWalkHttpAuthHandlerBase();

  // Provides an 'escape-hatch' out to Java for the browser/ layer
  // XWalkLoginDelegate.
  virtual bool HandleOnUIThread(content::WebContents*) = 0;
};

}  // namespace xwalk

#endif  // ANDROID_WEBVIEW_BROWSER_AW_HTTP_AUTH_HANDLER_BASE_H_
