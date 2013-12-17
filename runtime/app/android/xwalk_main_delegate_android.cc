// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/android/xwalk_main_delegate_android.h"

#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/posix/global_descriptors.h"
#include "content/public/browser/browser_main_runner.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/ui_base_switches.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
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
  int pak_fd =
      base::GlobalDescriptors::GetInstance()->MaybeGet(kXWalkPakDescriptor);
  if (pak_fd != base::kInvalidPlatformFileValue) {
    ui::ResourceBundle::InitSharedInstanceWithPakFile(pak_fd, false);
    ResourceBundle::GetSharedInstance().AddDataPackFromFile(
        pak_fd, ui::SCALE_FACTOR_100P);
    return;
  }
  base::FilePath pak_file;
  base::FilePath pak_dir;
  bool got_path = PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_dir);
  DCHECK(got_path);
  pak_dir = pak_dir.Append(FILE_PATH_LITERAL("paks"));
  pak_file = pak_dir.Append(FILE_PATH_LITERAL(kXWalkPakFilePath));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
}

}  // namespace xwalk
