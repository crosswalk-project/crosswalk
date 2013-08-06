// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "content/public/app/content_main.h"
#include "content/public/common/content_switches.h"
#include "xwalk/runtime/app/tizen/runtime_main.h"
#include "xwalk/runtime/app/xwalk_main_delegate.h"

int SubprocessMain(int argc, const char** argv) {
  xwalk::XWalkMainDelegate delegate;
  return content::ContentMain(argc, argv, &delegate);
}

int main(int argc, const char** argv) {
  CommandLine::Init(argc, argv);
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line.GetSwitchValueASCII(switches::kProcessType);
  if (process_type == "")
    return xwalk::RuntimeMain(argc, argv);

  return SubprocessMain(argc, argv);
}
