// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/common/shell_content_client.h"

#include "base/command_line.h"
#include "base/string_piece.h"
#include "base/utf_string_conversions.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/common/content_switches.h"
#include "grit/cameo_resources.h"
#include "grit/webkit_resources.h"
#include "grit/webkit_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "webkit/user_agent/user_agent_util.h"

namespace content {

ShellContentClient::~ShellContentClient() {
}

std::string ShellContentClient::GetUserAgent() const {
  std::string product = "Chrome/" CONTENT_SHELL_VERSION;
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kUseMobileUserAgent))
    product += " Mobile";
  return webkit_glue::BuildUserAgentFromProduct(product);
}

string16 ShellContentClient::GetLocalizedString(int message_id) const {
  return l10n_util::GetStringUTF16(message_id);
}

base::StringPiece ShellContentClient::GetDataResource(
    int resource_id,
    ui::ScaleFactor scale_factor) const {
  return ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedStaticMemory* ShellContentClient::GetDataResourceBytes(
    int resource_id) const {
  return ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

gfx::Image& ShellContentClient::GetNativeImageNamed(int resource_id) const {
  return ResourceBundle::GetSharedInstance().GetNativeImageNamed(resource_id);
}

}  // namespace content
