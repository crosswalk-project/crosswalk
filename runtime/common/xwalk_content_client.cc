// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_content_client.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "components/nacl/common/nacl_process_type.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/user_agent.h"
#include "content/public/common/pepper_plugin_info.h"
#include "ppapi/shared_impl/ppapi_permissions.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "xwalk/runtime/common/xwalk_paths.h"

const char* const xwalk::XWalkContentClient::kNaClPluginName = "Native Client";

namespace {

const char kNaClPluginMimeType[] = "application/x-nacl";
const char kNaClPluginExtension[] = "";
const char kNaClPluginDescription[] = "Native Client Executable";
const uint32 kNaClPluginPermissions = ppapi::PERMISSION_PRIVATE |
                                      ppapi::PERMISSION_DEV;

const char kPnaclPluginMimeType[] = "application/x-pnacl";
const char kPnaclPluginExtension[] = "";
const char kPnaclPluginDescription[] = "Portable Native Client Executable";

}  // namespace

namespace xwalk {

std::string GetProduct() {
  return "Chrome/" CHROME_VERSION;
}

std::string GetUserAgent() {
  std::string product = GetProduct();
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

void XWalkContentClient::AddPepperPlugins(
    std::vector<content::PepperPluginInfo>* plugins) {
  // Handle Native Client just like the PDF plugin. This means that it is
  // enabled by default for the non-portable case.  This allows apps installed
  // from the Chrome Web Store to use NaCl even if the command line switch
  // isn't set.  For other uses of NaCl we check for the command line switch.
  // Specifically, Portable Native Client is only enabled by the command line
  // switch.
  static bool skip_nacl_file_check = false;
  base::FilePath path;
  if (PathService::Get(xwalk::FILE_NACL_PLUGIN, &path)) {
    if (skip_nacl_file_check || base::PathExists(path)) {
      content::PepperPluginInfo nacl;
      nacl.path = path;
      nacl.name = XWalkContentClient::kNaClPluginName;
      content::WebPluginMimeType nacl_mime_type(kNaClPluginMimeType,
                                                kNaClPluginExtension,
                                                kNaClPluginDescription);
      nacl.mime_types.push_back(nacl_mime_type);
      if (!CommandLine::ForCurrentProcess()->HasSwitch(
              switches::kDisablePnacl)) {
        content::WebPluginMimeType pnacl_mime_type(kPnaclPluginMimeType,
                                                   kPnaclPluginExtension,
                                                   kPnaclPluginDescription);
        nacl.mime_types.push_back(pnacl_mime_type);
      }
      nacl.permissions = kNaClPluginPermissions;
      plugins->push_back(nacl);

      skip_nacl_file_check = true;
    }
  }
}

std::string XWalkContentClient::GetProduct() const {
  return xwalk::GetProduct();
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

std::string XWalkContentClient::GetProcessTypeNameInEnglish(int type) {
  switch (type) {
    case PROCESS_TYPE_NACL_LOADER:
      return "Native Client module";
    case PROCESS_TYPE_NACL_BROKER:
      return "Native Client broker";
  }

  DCHECK(false) << "Unknown child process type!";
  return "Unknown";
}

}  // namespace xwalk
