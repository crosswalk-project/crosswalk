// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell_browser_main.h"

#include "base/message_loop.h"
#include "content/public/browser/browser_main_runner.h"

// Main routine for running as the Browser process.
int ShellBrowserMain(const content::MainFunctionParams& parameters) {
  scoped_ptr<content::BrowserMainRunner> main_runner_(
      content::BrowserMainRunner::Create());

  int exit_code = main_runner_->Initialize(parameters);

  if (exit_code >= 0)
    return exit_code;

  exit_code = main_runner_->Run();

  main_runner_->Shutdown();

  return exit_code;
}
