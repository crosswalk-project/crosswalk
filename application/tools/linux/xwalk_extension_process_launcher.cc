// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"
#include "base/message_loop/message_loop.h"
#include "xwalk/application/tools/linux/xwalk_extension_process_launcher.h"
#include "xwalk/extensions/extension_process/xwalk_extension_process.h"

XWalkExtensionProcessLauncher::XWalkExtensionProcessLauncher()
    : base::Thread("LauncherExtensionService"),
      is_started_(false) {
  exit_manager_.reset(new base::AtExitManager);
  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_DEFAULT;
  StartWithOptions(thread_options);
}

XWalkExtensionProcessLauncher::~XWalkExtensionProcessLauncher() {
  Stop();
}

void XWalkExtensionProcessLauncher::CleanUp() {
  extension_process_.reset();
}

void XWalkExtensionProcessLauncher::Launch(
    const std::string& channel_id, int channel_fd) {
  is_started_ = true;
  message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&XWalkExtensionProcessLauncher::StartExtensionProcess,
                 base::Unretained(this), channel_id, channel_fd));
}

void XWalkExtensionProcessLauncher::StartExtensionProcess(
    const std::string& channel_id, int channel_fd) {
  extension_process_.reset(new xwalk::extensions::XWalkExtensionProcess(
      IPC::ChannelHandle(channel_id, base::FileDescriptor(channel_fd, true))));
}
