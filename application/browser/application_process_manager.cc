// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <string>
#include "base/bind.h"
#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/time.h"
#include "base/memory/linked_ptr.h"
#include "base/perftimer.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "net/base/net_util.h"

using content::WebContents;
using xwalk::Runtime;
using xwalk::RuntimeContext;

namespace xwalk_application {

ApplicationProcessManager::ApplicationProcessManager(
    RuntimeContext* runtime_context)
    : weak_ptr_factory_(this) {
}

ApplicationProcessManager::~ApplicationProcessManager() {
}

void ApplicationProcessManager::LaunchApplication(
        RuntimeContext* runtime_context,
        const Application* application) {
  std::string start_page;
  application->manifest()->GetString(
      application_manifest_keys::kLaunchLocalPath,
      &start_page);

  if (start_page.empty()) {
    // TODO(Xinchao): when there's no start page in manifest.json, please add a
    // default one.
  }

  GURL startup_url = net::FilePathToFileURL(
      application->path().Append(start_page));

  Runtime::Create(runtime_context, startup_url);
}

}  // namespace xwalk_application
