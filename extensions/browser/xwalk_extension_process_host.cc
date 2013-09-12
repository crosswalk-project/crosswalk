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
#include "content/public/browser/render_process_host.h"
#include "content/public/common/child_process_host.h"
#include "content/public/common/process_type.h"
#include "content/public/common/content_switches.h"
#if defined(OS_WIN)
#include "content/public/common/sandboxed_process_launcher_delegate.h"
#endif
#include "ipc/ipc_message.h"
#include "ipc/ipc_switches.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"

using content::BrowserThread;

namespace xwalk {
namespace extensions {

#if defined(OS_WIN)
class ExtensionSandboxedProcessLauncherDelegate
    : public content::SandboxedProcessLauncherDelegate {
 public:
  ExtensionSandboxedProcessLauncherDelegate() {}
  virtual ~ExtensionSandboxedProcessLauncherDelegate() {}

  virtual void ShouldSandbox(bool* in_sandbox) OVERRIDE {
    *in_sandbox = false;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ExtensionSandboxedProcessLauncherDelegate);
};
#endif

XWalkExtensionProcessHost::XWalkExtensionProcessHost()
    : ep_rp_channel_handle_(""),
      render_process_host_(0),
      is_extension_process_channel_ready_(false) {
  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(&XWalkExtensionProcessHost::StartProcess,
      base::Unretained(this)));
}

XWalkExtensionProcessHost::~XWalkExtensionProcessHost() {
  // FIXME(jeez): We have to find a way to handle a ^C on Linux,
  // in order to avoid leaving zombies behind. I couldn't find any
  // content/public/ way of handling this, but Chrome does some trickery
  // at chrome/browser/chrome_browser_main_posix.cc .
  StopProcess();
}

void XWalkExtensionProcessHost::StartProcess() {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  CHECK(!process_);

  process_.reset(content::BrowserChildProcessHost::Create(
      content::PROCESS_TYPE_CONTENT_END, this));

  std::string channel_id = process_->GetHost()->CreateChannel();
  CHECK(!channel_id.empty());

  base::FilePath exe_path = content::ChildProcessHost::GetChildPath(
      content::ChildProcessHost::CHILD_NORMAL);
  scoped_ptr<CommandLine> cmd_line(new CommandLine(exe_path));
  cmd_line->AppendSwitchASCII(switches::kProcessType,
                              switches::kXWalkExtensionProcess);
  cmd_line->AppendSwitchASCII(switches::kProcessChannelID, channel_id);
  process_->Launch(
#if defined(OS_WIN)
      new ExtensionSandboxedProcessLauncherDelegate(),
#elif defined(OS_POSIX)
    false, base::EnvironmentVector(),
#endif
    cmd_line.release());
}

void XWalkExtensionProcessHost::StopProcess() {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  CHECK(process_);

  process_.reset();
}

void XWalkExtensionProcessHost::RegisterExternalExtensions(
    const base::FilePath& extension_path) {
  Send(new XWalkExtensionProcessMsg_RegisterExtensions(extension_path));
}

void XWalkExtensionProcessHost::OnRenderProcessHostCreated(
    content::RenderProcessHost* render_process_host) {
  render_process_host_ = render_process_host;
  CHECK(render_process_host_);

  SendChannelHandleToRenderProcess();
}

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
    IPC_MESSAGE_HANDLER(
        XWalkExtensionProcessHostMsg_RenderProcessChannelCreated,
        OnRenderChannelCreated)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionProcessHost::OnProcessCrashed(int exit_code) {
  VLOG(1) << "Process crashed with exit_code=" << exit_code;
}

void XWalkExtensionProcessHost::OnProcessLaunched() {
  VLOG(1) << "\n\nExtensionProcess was started!";
}

void XWalkExtensionProcessHost::OnRenderChannelCreated(
    const IPC::ChannelHandle& handle) {
  ep_rp_channel_handle_ = handle;
  is_extension_process_channel_ready_ = true;

  SendChannelHandleToRenderProcess();
}

void XWalkExtensionProcessHost::SendChannelHandleToRenderProcess() {
  // It can be that the EP channel got created before the RenderProcessHost.
  if (!render_process_host_)
    return;

  // It can be that the RenderProcessHost got created before the EP channel.
  if (!is_extension_process_channel_ready_)
    return;

  render_process_host_->Send(new XWalkViewMsg_ExtensionProcessChannelCreated(
      ep_rp_channel_handle_));
}


}  // namespace extensions
}  // namespace xwalk

