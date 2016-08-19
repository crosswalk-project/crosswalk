// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_content_client.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "components/nacl/common/nacl_process_type.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/user_agent.h"
#if defined(ENABLE_PLUGINS)
#include "content/public/common/pepper_plugin_info.h"
#include "ppapi/shared_impl/ppapi_permissions.h"
#endif
#if !defined(DISABLE_NACL)
#include "components/nacl/renderer/plugin/ppapi_entrypoints.h"
#endif
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "xwalk/runtime/common/xwalk_paths.h"

const char* const xwalk::XWalkContentClient::kNaClPluginName = "Native Client";

namespace {

#if !defined(DISABLE_NACL)
const char kNaClPluginMimeType[] = "application/x-nacl";
const char kNaClPluginExtension[] = "";
const char kNaClPluginDescription[] = "Native Client Executable";
const uint32_t kNaClPluginPermissions = ppapi::PERMISSION_PRIVATE |
                                      ppapi::PERMISSION_DEV;

const char kPnaclPluginMimeType[] = "application/x-pnacl";
const char kPnaclPluginExtension[] = "";
const char kPnaclPluginDescription[] = "Portable Native Client Executable";
#endif

#if defined(ENABLE_PLUGINS)
const int32_t kPepperFlashPermissions = ppapi::PERMISSION_DEV |
                                      ppapi::PERMISSION_PRIVATE |
                                      ppapi::PERMISSION_BYPASS_USER_GESTURE |
                                      ppapi::PERMISSION_FLASH;

content::PepperPluginInfo CreatePepperFlashInfo(const base::FilePath& path,
                                                const std::string& version) {
  content::PepperPluginInfo plugin;

  plugin.is_out_of_process = true;
  plugin.name = content::kFlashPluginName;
  plugin.path = path;
  plugin.permissions = kPepperFlashPermissions;

  std::vector<std::string> flash_version_numbers = base::SplitString(
      version, ".", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
  if (flash_version_numbers.size() < 1)
    flash_version_numbers.push_back("11");
  // |SplitString()| puts in an empty string given an empty string. :(
  else if (flash_version_numbers[0].empty())
    flash_version_numbers[0] = "11";
  if (flash_version_numbers.size() < 2)
    flash_version_numbers.push_back("2");
  if (flash_version_numbers.size() < 3)
    flash_version_numbers.push_back("999");
  if (flash_version_numbers.size() < 4)
    flash_version_numbers.push_back("999");
  // E.g., "Shockwave Flash 10.2 r154":
  plugin.description = plugin.name + " " + flash_version_numbers[0] + "." +
      flash_version_numbers[1] + " r" + flash_version_numbers[2];
  plugin.version = base::JoinString(flash_version_numbers, ".");
  content::WebPluginMimeType swf_mime_type(content::kFlashPluginSwfMimeType,
                                           content::kFlashPluginSwfExtension,
                                           content::kFlashPluginSwfDescription);
  plugin.mime_types.push_back(swf_mime_type);
  content::WebPluginMimeType spl_mime_type(content::kFlashPluginSplMimeType,
                                           content::kFlashPluginSplExtension,
                                           content::kFlashPluginSplDescription);
  plugin.mime_types.push_back(spl_mime_type);

  return plugin;
}

void AddPepperFlashFromCommandline(
    std::vector<content::PepperPluginInfo>* plugins) {
  const base::CommandLine::StringType path =
      base::CommandLine::ForCurrentProcess()->GetSwitchValueNative(
          switches::kPpapiFlashPath);
  if (path.empty())
    return;

  // Also get the version from the command-line. Should be something like 11.2
  // or 11.2.123.45.
  std::string version =
      base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          switches::kPpapiFlashVersion);

  plugins->push_back(
      CreatePepperFlashInfo(base::FilePath(path), version));
}
#endif

}  // namespace

namespace xwalk {

std::string GetProduct() {
  return "Chrome/" CHROME_VERSION;
}

std::string GetUserAgent() {
  std::string product = GetProduct();
  product += " Crosswalk/" XWALK_VERSION;
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
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
#if !defined(DISABLE_NACL)
  // Handle Native Client just like the PDF plugin. This means that it is
  // enabled by default for the non-portable case.  This allows apps installed
  // from the Chrome Web Store to use NaCl even if the command line switch
  // isn't set.  For other uses of NaCl we check for the command line switch.
  // Specifically, Portable Native Client is only enabled by the command line
  // switch.
  base::FilePath path;
  if (PathService::Get(xwalk::FILE_NACL_PLUGIN, &path)) {
    content::PepperPluginInfo nacl;
    // The nacl plugin is now built into the Chromium binary.
    nacl.is_internal = true;
    nacl.path = path;
    nacl.name = XWalkContentClient::kNaClPluginName;
    content::WebPluginMimeType nacl_mime_type(kNaClPluginMimeType,
                                              kNaClPluginExtension,
                                              kNaClPluginDescription);
    nacl.mime_types.push_back(nacl_mime_type);
    if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kDisablePnacl)) {
      content::WebPluginMimeType pnacl_mime_type(kPnaclPluginMimeType,
                                                 kPnaclPluginExtension,
                                                 kPnaclPluginDescription);
      nacl.mime_types.push_back(pnacl_mime_type);
    }
    nacl.internal_entry_points.get_interface = nacl_plugin::PPP_GetInterface;
    nacl.internal_entry_points.initialize_module =
        nacl_plugin::PPP_InitializeModule;
    nacl.internal_entry_points.shutdown_module =
        nacl_plugin::PPP_ShutdownModule;
    nacl.permissions = kNaClPluginPermissions;
    plugins->push_back(nacl);
  }
#endif

#if defined(ENABLE_PLUGINS)
  AddPepperFlashFromCommandline(plugins);
#endif
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

base::RefCountedMemory* XWalkContentClient::GetDataResourceBytes(
    int resource_id) const {
  return ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

gfx::Image& XWalkContentClient::GetNativeImageNamed(int resource_id) const {
  return ResourceBundle::GetSharedInstance().GetNativeImageNamed(resource_id);
}

void XWalkContentClient::AddAdditionalSchemes(
    std::vector<url::SchemeWithType>* standard_schemes,
    std::vector<url::SchemeWithType>* referrer_schemes,
    std::vector<std::string>* savable_schemes) {
  url::SchemeWithType app_scheme = {application::kApplicationScheme,
                                    url::SCHEME_WITHOUT_PORT};
  standard_schemes->push_back(app_scheme);
  savable_schemes->push_back(application::kApplicationScheme);
}

void XWalkContentClient::AddSecureSchemesAndOrigins(
    std::set<std::string>* schemes,
    std::set<GURL>* origins) {
    schemes->insert(application::kApplicationScheme);
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
