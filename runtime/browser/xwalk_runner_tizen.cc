// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner_tizen.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "crypto/nss_util.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/runtime/browser/sysapps_component.h"
#include "xwalk/runtime/browser/xwalk_component.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {

XWalkRunnerTizen::XWalkRunnerTizen() {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  shared_process_mode_enabled_ =
      !(cmd_line->HasSwitch(switches::kXWalkDisableSharedProcessMode));
#if defined(USE_CYNARA)
  // Initialize Cynara
  if (cynara_initialize(&cynara_handler_, cynara_conf_) != CYNARA_API_SUCCESS) {
    // TODO(Peter Wang): Handle degrading/exit-with-error if cynara fails
    //       to initialize.
    //       For now, log an error.
    LOG(ERROR) << "Cynara failed to initialize.";
  }
#endif
}

XWalkRunnerTizen::~XWalkRunnerTizen() {
#if defined(USE_CYNARA)
  // Finish with cynara
  cynara_finish(cynara_handler_);
#endif
}

// static
XWalkRunnerTizen* XWalkRunnerTizen::GetInstance() {
  return static_cast<XWalkRunnerTizen*>(XWalkRunner::GetInstance());
}

void XWalkRunnerTizen::PreMainMessageLoopRun() {
  XWalkRunner::PreMainMessageLoopRun();

  // NSSInitSingleton is a costly operation (up to 100ms on VTC-1010),
  // resulting in postponing the parsing and composition steps of the render
  // process at cold start. Therefore, move the initialization logic here.
  if (shared_process_mode_enabled()) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&crypto::EnsureNSSInit));
  }
}

}  // namespace xwalk
