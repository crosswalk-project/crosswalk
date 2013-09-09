// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_process_host.h"

#include <string>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/files/file_path.h"
#include "content/public/browser/browser_child_process_host.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/child_process_host.h"
#include "content/public/common/process_type.h"
#include "content/public/common/content_switches.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_switches.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"

using content::BrowserThread;

namespace xwalk {
namespace extensions {

XWalkExtensionProcessHost::XWalkExtensionProcessHost() {
  StartProcess();
}

XWalkExtensionProcessHost::~XWalkExtensionProcessHost() {
  // FIXME(jeez): We have to find a way to handle a ^C on Linux,
  // in order to avoid leaving zombies behind. I couldn't find any
  // content/public/ way of handling this, but Chrome does some trickery
  // at chrome/browser/chrome_browser_main_posix.cc .
  StopProcess();
}

void XWalkExtensionProcessHost::StartProcess() {
  CHECK(!process_);

  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&XWalkExtensionProcessHost::StartProcess,
                 base::Unretained(this)));
    return;
  }

  process_.reset(content::BrowserChildProcessHost::Create(
      content::PROCESS_TYPE_CONTENT_END, this));

  std::string channel_id = process_->GetHost()->CreateChannel();
  if (channel_id.empty()) {
    LOG(ERROR) << "Extension process launch failed: could not create channel";
    return;
  }

  base::FilePath exe_path = content::ChildProcessHost::GetChildPath(
      content::ChildProcessHost::CHILD_NORMAL);
  scoped_ptr<CommandLine> cmd_line(new CommandLine(exe_path));
  cmd_line->AppendSwitchASCII(switches::kProcessType,
                              switches::kXWalkExtensionProcess);
  cmd_line->AppendSwitchASCII(switches::kProcessChannelID, channel_id);
  process_->Launch(false, base::EnvironmentVector(), cmd_line.release());
}

void XWalkExtensionProcessHost::StopProcess() {
  CHECK(process_);

  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&XWalkExtensionProcessHost::StopProcess,
                 base::Unretained(this)));
    return;
  }

  process_.reset();
}

void XWalkExtensionProcessHost::RegisterExternalExtensions(
    const base::FilePath& extension_path) {
  Send(new XWalkExtensionProcessMsg_RegisterExtensions(extension_path));
}

void XWalkExtensionProcessHost::OnRenderProcessHostCreated(
    content::RenderProcessHost* render_process_host) {}

void XWalkExtensionProcessHost::Send(IPC::Message* msg) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
        base::Bind(&XWalkExtensionProcessHost::Send,
        base::Unretained(this), msg));
    return;
  }

  process_->GetHost()->Send(msg);
}

bool XWalkExtensionProcessHost::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionProcessHost, message)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionProcessHost::OnProcessCrashed(int exit_code) {
  VLOG(0) << "Process crashed with exit_code=" << exit_code;
}

void XWalkExtensionProcessHost::OnProcessLaunched() {
  VLOG(0) << "\n\nExtensionProcess was started!";
}

}  // namespace extensions
}  // namespace xwalk

