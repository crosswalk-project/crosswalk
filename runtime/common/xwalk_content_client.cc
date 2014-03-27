// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_content_client.h"

#include "base/command_line.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/user_agent.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/common/constants.h"

namespace xwalk {

std::string GetUserAgent() {
  std::string product = "Chrome/" CHROME_VERSION;
#if (defined(OS_TIZEN) || defined(OS_ANDROID))
  product += " Mobile Crosswalk/" XWALK_VERSION;
#else
  product += " Crosswalk/" XWALK_VERSION;
#endif
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kUseMobileUserAgent))
    product += " Mobile";
  return content::BuildUserAgentFromProduct(product);
}

XWalkContentClient::XWalkContentClient() {
}

XWalkContentClient::~XWalkContentClient() {
  xwalk::GetUserAgent();
}

std::string XWalkContentClient::GetProduct() const {
  return std::string("Version/4.0");
}

std::string XWalkContentClient::GetUserAgent() const {
  return xwalk::GetUserAgent();
}

base::string16 XWalkContentClient::GetLocalizedString(int message_id) const {
  return l10n_util::GetStringUTF16(message_id);
}

base::StringPiece XWalkContentClient::GetDataResource(
    int resource_id,
    ui::ScaleFactor scale_factor) const {
  return ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedStaticMemory* XWalkContentClient::GetDataResourceBytes(
    int resource_id) const {
  return ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

gfx::Image& XWalkContentClient::GetNativeImageNamed(int resource_id) const {
  return ResourceBundle::GetSharedInstance().GetNativeImageNamed(resource_id);
}

void XWalkContentClient::AddAdditionalSchemes(
    std::vector<std::string>* standard_schemes,
    std::vector<std::string>* savable_schemes) {
  standard_schemes->push_back(application::kApplicationScheme);
  savable_schemes->push_back(application::kApplicationScheme);
}

}  // namespace xwalk
