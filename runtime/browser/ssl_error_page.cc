// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ssl_error_page.h"

#include "base/strings/string_number_conversions.h"
#include "content/public/browser/interstitial_page.h"
#include "content/public/browser/web_contents.h"

using content::WebContents;
using net::SSLInfo;

namespace xwalk {

enum UserCommandsFromPage {
  CMD_DONT_PROCEED = 0,
  CMD_PROCEED = 1,
};

SSLErrorPage::SSLErrorPage(WebContents* web_contents,
                           int cert_error,
                           const SSLInfo& ssl_info,
                           const GURL& request_url,
                           const base::Callback<void(bool)>& callback)
    : web_contents_(web_contents),
      ssl_info_(ssl_info),
      request_url_(request_url),
      callback_(callback),
      interstitial_page_(nullptr) {
}

SSLErrorPage::~SSLErrorPage() {
}

void SSLErrorPage::Show() {
  DCHECK(!interstitial_page_);
  interstitial_page_ = content::InterstitialPage::Create(
      web_contents_, true, request_url_, this);
  interstitial_page_->Show();
}

void SSLErrorPage::OnProceed() {
  // Allow certificate
  callback_.Run(true);
  callback_.Reset();
}

void SSLErrorPage::OnDontProceed() {
  // Deny certificate
  callback_.Run(false);
  callback_.Reset();
}

void SSLErrorPage::CommandReceived(const std::string& command) {
  int cmd = 0;
  bool retval = base::StringToInt(command, &cmd);
  DCHECK(retval);
  switch (cmd) {
    case CMD_DONT_PROCEED:
      interstitial_page_->DontProceed();
      break;
    case CMD_PROCEED:
      interstitial_page_->Proceed();
      break;
    default:
      break;
  }
}

// TODO(Peter Wang): Provide a more user-friendly page including
// icons, localized strings, and the details of SSL error etc.
std::string SSLErrorPage::GetHTMLContents() {
  std::string proceed_link =
      "<p><a href=\"javascript:proceed()\"> Proceed to " +
      request_url_.spec() + "</a></p>";
  return "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">"
         "<html><head><title>Untrusted Connection</title>"
         "<script>function proceed() {"
         "window.domAutomationController.setAutomationId(1);"
         "window.domAutomationController.send(1);}"
         "</script></head>"
         "<body><h1>This Connection is Untrusted</h1>"
         "<p>This site uses an invalid security certificate.</p>"
         "<p>Close or proceed if your understand the risks</p>" +
         proceed_link +
         "</body></html>";
}

}  // namespace xwalk
