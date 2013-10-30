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

// This filter is used by ExtensionProcessHost to intercept when Render Process
// ask for the Extension Channel handle (that is created by extension process).
class XWalkExtensionProcessHost::RenderProcessMessageFilter
    : public IPC::ChannelProxy::MessageFilter {
 public:
  explicit RenderProcessMessageFilter(XWalkExtensionProcessHost* eph)
      : eph_(eph) {}

  // This exists to fulfill the requirement for delayed reply handling, since it
  // needs to send a message back if the parameters couldn't be correctly read
  // from the original message received. See DispatchDealyReplyWithSendParams().
  bool Send(IPC::Message* message) {
    return eph_->render_process_host_->Send(message);
  }

 private:
  // IPC::ChannelProxy::MessageFilter implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE {
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(RenderProcessMessageFilter, message)
      IPC_MESSAGE_HANDLER_DELAY_REPLY(
          XWalkExtensionProcessHostMsg_GetExtensionProcessChannel,
          OnGetExtensionProcessChannel)
      IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;
  }

  void OnGetExtensionProcessChannel(IPC::Message* reply) {
    scoped_ptr<IPC::Message> scoped_reply(reply);
    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&XWalkExtensionProcessHost::OnGetExtensionProcessChannel,
                   base::Unretained(eph_),
                   base::Passed(&scoped_reply)));
  }

  virtual ~RenderProcessMessageFilter() {}
  XWalkExtensionProcessHost* eph_;
};

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

XWalkExtensionProcessHost::XWalkExtensionProcessHost(
    content::RenderProcessHost* render_process_host,
    const base::FilePath& external_extensions_path)
    : ep_rp_channel_handle_(""),
      render_process_host_(render_process_host),
      render_process_message_filter_(new RenderProcessMessageFilter(this)),
      external_extensions_path_(external_extensions_path),
      is_extension_process_channel_ready_(false) {
  render_process_host_->GetChannel()->AddFilter(render_process_message_filter_);
  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(&XWalkExtensionProcessHost::StartProcess,
      base::Unretained(this)));
}

XWalkExtensionProcessHost::~XWalkExtensionProcessHost() {
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
    false, base::EnvironmentMap(),
#endif
    cmd_line.release());

  process_->GetHost()->Send(new XWalkExtensionProcessMsg_RegisterExtensions(
      external_extensions_path_));
}

void XWalkExtensionProcessHost::StopProcess() {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  CHECK(process_);

  process_.reset();
}

void XWalkExtensionProcessHost::OnGetExtensionProcessChannel(
    scoped_ptr<IPC::Message> reply) {
  pending_reply_for_render_process_ = reply.Pass();
  ReplyChannelHandleToRenderProcess();

  // We just need to send the channel information once, so the filter is no
  // longer necessary.
  render_process_host_->GetChannel()->RemoveFilter(
      render_process_message_filter_);
  render_process_message_filter_ = NULL;
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
  is_extension_process_channel_ready_ = true;
  ep_rp_channel_handle_ = handle;
  ReplyChannelHandleToRenderProcess();
}

void XWalkExtensionProcessHost::ReplyChannelHandleToRenderProcess() {
  // Replying the channel handle to RP depends on two events:
  // - EP already notified EPH that new channel was created (for RP<->EP).
  // - RP already asked for the channel handle.
  //
  // The order for this events is not determined, so we call this function from
  // both, and the second execution will send the reply.
  if (!is_extension_process_channel_ready_
      || !pending_reply_for_render_process_)
    return;

  IPC::WriteParam(pending_reply_for_render_process_.get(),
                  ep_rp_channel_handle_);
  render_process_host_->Send(pending_reply_for_render_process_.release());
}


}  // namespace extensions
}  // namespace xwalk

