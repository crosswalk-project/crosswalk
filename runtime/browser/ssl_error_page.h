// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_SSL_ERROR_PAGE_H_
#define XWALK_RUNTIME_BROWSER_SSL_ERROR_PAGE_H_

#include <string>

#include "base/callback.h"
#include "content/public/browser/interstitial_page_delegate.h"
#include "net/ssl/ssl_info.h"
#include "url/gurl.h"

namespace content {
class InterstitialPage;
class WebContents;
}

namespace xwalk {

class SSLErrorPage : public content::InterstitialPageDelegate {
 public:
  SSLErrorPage(content::WebContents* web_contents,
               int cert_error,
               const net::SSLInfo& ssl_info,
               const GURL& request_url,
               const base::Callback<void(bool)>& callback);

  ~SSLErrorPage() override;

  // Show an interstitial page to let user to choose the next action
  void Show();

 protected:
  // Overridden methods of InterstitialPageDelegate
  void OnProceed() override;
  void OnDontProceed() override;
  void CommandReceived(const std::string& command) override;
  std::string GetHTMLContents() override;

 private:
  content::WebContents* web_contents_;
  const net::SSLInfo ssl_info_;
  const GURL request_url_;
  base::Callback<void(bool)> callback_;
  content::InterstitialPage* interstitial_page_;

  DISALLOW_COPY_AND_ASSIGN(SSLErrorPage);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_SSL_ERROR_PAGE_H_
