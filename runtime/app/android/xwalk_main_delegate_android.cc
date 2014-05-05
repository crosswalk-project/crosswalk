// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/android/xwalk_main_delegate_android.h"

#include <string>

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/base_paths_android.h"
#include "base/path_service.h"
#include "base/platform_file.h"
#include "base/posix/global_descriptors.h"
#include "base/strings/string_util.h"
#include "content/public/browser/browser_main_runner.h"
#include "ui/base/l10n/l10n_util_android.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/ui_base_switches.h"
#include "xwalk/runtime/common/android/xwalk_globals_android.h"
#include "xwalk/runtime/common/xwalk_content_client.h"

namespace xwalk {

XWalkMainDelegateAndroid::XWalkMainDelegateAndroid() {
}

XWalkMainDelegateAndroid::~XWalkMainDelegateAndroid() {
}

bool XWalkMainDelegateAndroid::BasicStartupComplete(int* exit_code) {
  content_client_.reset(new XWalkContentClient);
  content::SetContentClient(content_client_.get());
  return false;
}

void XWalkMainDelegateAndroid::PreSandboxStartup() {
  InitResourceBundle();
}

int XWalkMainDelegateAndroid::RunProcess(
    const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (process_type.empty()) {
    browser_runner_.reset(content::BrowserMainRunner::Create());
    int exit_code = browser_runner_->Initialize(main_function_params);
    DCHECK_LT(exit_code, 0);

    return 0;
  }
  return -1;
}

void XWalkMainDelegateAndroid::InitResourceBundle() {
  base::FilePath pak_file;
  base::FilePath pak_dir;
  bool got_path = PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_dir);
  DCHECK(got_path);
  pak_dir = pak_dir.Append(FILE_PATH_LITERAL("paks"));
  pak_file = pak_dir.Append(FILE_PATH_LITERAL(kXWalkPakFilePath));

  std::string locale = l10n_util::GetDefaultLocale();
  base::FilePath locale_pak_file;
  base::File locale_pak_fd;
  int flags = base::File::FLAG_OPEN | base::File::FLAG_READ;
  if ( !StartsWithASCII(locale, "zh-", true)
      && !StartsWithASCII(locale, "en-", true)
      && !StartsWithASCII(locale, "pt-", true)) {
    // For languages other than "zh-*", "en-*" and "pt-*", the country code
    // is not required, so remove the country code here if provided.
    size_t delim_index = locale.find("-");
    if (delim_index != std::string::npos) {
      locale = locale.substr(0, delim_index);
    }
  }

  locale = locale + std::string(".pak");
  locale_pak_file = pak_dir.Append(FILE_PATH_LITERAL(locale));
  locale_pak_fd.Initialize(locale_pak_file, flags);

  if (!locale_pak_fd.IsValid()) {
    // If the language pak is not available, just fallback to use default
    // locale that is packaged in kXWalkPakFilePath.
    ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
  } else {
    // Initialize locale resource pak.
    ui::ResourceBundle::InitSharedInstanceWithPakPath(locale_pak_file);
    // Add the XWalk data pak.
    ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
        pak_file, ui::SCALE_FACTOR_100P);
  }

  locale_pak_fd.Close();
}

}  // namespace xwalk
