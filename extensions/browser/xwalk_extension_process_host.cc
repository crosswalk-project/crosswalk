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
#include "content/public/common/sandboxed_process_launcher_delegate.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_switches.h"
#include "ipc/message_filter.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/runtime/common/xwalk_switches.h"

using content::BrowserThread;

namespace xwalk {
namespace extensions {

// This filter is used by ExtensionProcessHost to intercept when Render Process
// ask for the Extension Channel handle (that is created by extension process).
class XWalkExtensionProcessHost::RenderProcessMessageFilter
    : public IPC::MessageFilter {
 public:
  explicit RenderProcessMessageFilter(XWalkExtensionProcessHost* eph)
      : eph_(eph) {}

  // This exists to fulfill the requirement for delayed reply handling, since it
  // needs to send a message back if the parameters couldn't be correctly read
  // from the original message received. See DispatchDealyReplyWithSendParams().
  bool Send(IPC::Message* message) {
    if (eph_)
      return eph_->render_process_host_->Send(message);
    delete message;
    return false;
  }

  void Invalidate() {
    eph_ = NULL;
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
    if (eph_)
      eph_->OnGetExtensionProcessChannel(scoped_reply.Pass());
  }

  virtual ~RenderProcessMessageFilter() {}

  XWalkExtensionProcessHost* eph_;
};

class ExtensionSandboxedProcessLauncherDelegate
    : public content::SandboxedProcessLauncherDelegate {
 public:
  explicit ExtensionSandboxedProcessLauncherDelegate(
      content::ChildProcessHost* host)
#if defined(OS_POSIX)
      : ipc_fd_(host->TakeClientFileDescriptor())
#endif
  {}
  virtual ~ExtensionSandboxedProcessLauncherDelegate() {}

#if defined(OS_WIN)
  virtual bool ShouldSandbox() OVERRIDE {
    return false;
  }
#elif defined(OS_POSIX)
  virtual int GetIpcFd() OVERRIDE {
    return ipc_fd_;
  }
#endif

 private:
#if defined(OS_POSIX)
  int ipc_fd_;
#endif

  DISALLOW_COPY_AND_ASSIGN(ExtensionSandboxedProcessLauncherDelegate);
};

bool XWalkExtensionProcessHost::Delegate::OnRegisterPermissions(
    int render_process_id,
    const std::string& extension_name,
    const std::string& perm_table) {
  return false;
}

XWalkExtensionProcessHost::XWalkExtensionProcessHost(
    content::RenderProcessHost* render_process_host,
    const base::FilePath& external_extensions_path,
    XWalkExtensionProcessHost::Delegate* delegate,
    scoped_ptr<base::ValueMap> runtime_variables)
    : ep_rp_channel_handle_(""),
      render_process_host_(render_process_host),
      render_process_message_filter_(new RenderProcessMessageFilter(this)),
      external_extensions_path_(external_extensions_path),
      is_extension_process_channel_ready_(false),
      delegate_(delegate),
      runtime_variables_(runtime_variables.Pass()) {
  render_process_host_->GetChannel()->AddFilter(render_process_message_filter_);
  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(&XWalkExtensionProcessHost::StartProcess,
      base::Unretained(this)));
}

XWalkExtensionProcessHost::~XWalkExtensionProcessHost() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  render_process_message_filter_->Invalidate();
  StopProcess();
}

namespace {

void ToListValue(base::ValueMap* vm, base::ListValue* lv) {
  lv->Clear();

  for (base::ValueMap::iterator it = vm->begin(); it != vm->end(); it++) {
    base::DictionaryValue* dv = new base::DictionaryValue();
    dv->Set(it->first, it->second);
    lv->Append(dv);
  }
}

}  // namespace

void XWalkExtensionProcessHost::StartProcess() {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  CHECK(!process_ || !channel_);

#if defined(SHARED_PROCESS_MODE)
#if defined(OS_LINUX)
    std::string channel_id =
        IPC::Channel::GenerateVerifiedChannelID(std::string());
    channel_.reset(new IPC::Channel(
          channel_id, IPC::Channel::MODE_SERVER, this));
    if (!channel_->Connect())
      NOTREACHED();
    IPC::ChannelHandle channel_handle(channel_id,
        base::FileDescriptor(channel_->TakeClientFileDescriptor(), true));
    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(
            &XWalkExtensionProcessHost::Delegate::OnExtensionProcessCreated,
            base::Unretained(delegate_), render_process_host_->GetID(),
            channel_handle));
#else
    NOTIMPLEMENTED();
#endif  // #if defined(OS_LINUX)
#else
    process_.reset(content::BrowserChildProcessHost::Create(
        content::PROCESS_TYPE_CONTENT_END, this));

    std::string channel_id = process_->GetHost()->CreateChannel();
    CHECK(!channel_id.empty());

    CommandLine::StringType extension_cmd_prefix;
#if defined(OS_POSIX)
    const CommandLine &browser_command_line = *CommandLine::ForCurrentProcess();
    extension_cmd_prefix = browser_command_line.GetSwitchValueNative(
        switches::kXWalkExtensionCmdPrefix);
#endif

#if defined(OS_LINUX)
    int flags = extension_cmd_prefix.empty() ?
        content::ChildProcessHost::CHILD_ALLOW_SELF :
        content::ChildProcessHost::CHILD_NORMAL;
#else
    int flags = content::ChildProcessHost::CHILD_NORMAL;
#endif

    base::FilePath exe_path = content::ChildProcessHost::GetChildPath(flags);
    if (exe_path.empty())
      return;

    scoped_ptr<CommandLine> cmd_line(new CommandLine(exe_path));
    cmd_line->AppendSwitchASCII(switches::kProcessType,
                                switches::kXWalkExtensionProcess);
    cmd_line->AppendSwitchASCII(switches::kProcessChannelID, channel_id);
    if (!extension_cmd_prefix.empty())
      cmd_line->PrependWrapper(extension_cmd_prefix);

    process_->Launch(
        new ExtensionSandboxedProcessLauncherDelegate(process_->GetHost()),
        cmd_line.release());
#endif  // #if defined(SHARED_PROCESS_MODE)

  base::ListValue runtime_variables_lv;
  ToListValue(&const_cast<base::ValueMap&>(*runtime_variables_),
      &runtime_variables_lv);
  Send(new XWalkExtensionProcessMsg_RegisterExtensions(
        external_extensions_path_, runtime_variables_lv));
}

void XWalkExtensionProcessHost::StopProcess() {
  if (process_)
    process_.reset();
  if (channel_)
    channel_.reset();
}

void XWalkExtensionProcessHost::OnGetExtensionProcessChannel(
    scoped_ptr<IPC::Message> reply) {
  pending_reply_for_render_process_ = reply.Pass();
  ReplyChannelHandleToRenderProcess();
}

bool XWalkExtensionProcessHost::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionProcessHost, message)
    IPC_MESSAGE_HANDLER(
        XWalkExtensionProcessHostMsg_RenderProcessChannelCreated,
        OnRenderChannelCreated)
    IPC_MESSAGE_HANDLER_DELAY_REPLY(
        XWalkExtensionProcessHostMsg_CheckAPIAccessControl,
        OnCheckAPIAccessControl)
    IPC_MESSAGE_HANDLER(
        XWalkExtensionProcessHostMsg_RegisterPermissions,
        OnRegisterPermissions)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionProcessHost::OnChannelError() {
  // This function is called just before
  // BrowserChildProcessHostImpl::OnChildDisconnected gets called. Please refer
  // to ChildProcessHostImpl::OnChannelError() from child_process_host_impl.cc.
  // This means that content::BrowserChildProcessHost (process_, in our case)
  // is about to delete its delegate, which is us!
  // We should alert our XWalkExtensionProcessHost::Delegate, since it will
  // most likely have a pointer to us that needs to be invalidated.

  VLOG(1) << "\n\nExtensionProcess crashed";
  if (delegate_)
    delegate_->OnExtensionProcessDied(this, render_process_host_->GetID());
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

  XWalkExtensionProcessHostMsg_GetExtensionProcessChannel::WriteReplyParams(
      pending_reply_for_render_process_.get(), ep_rp_channel_handle_);

  render_process_host_->Send(pending_reply_for_render_process_.release());
}

void XWalkExtensionProcessHost::ReplyAccessControlToExtension(
    IPC::Message* reply_msg,
    RuntimePermission perm) {
  XWalkExtensionProcessHostMsg_CheckAPIAccessControl
      ::WriteReplyParams(reply_msg, perm);
  Send(reply_msg);
}

void XWalkExtensionProcessHost::OnCheckAPIAccessControl(
    const std::string& extension_name,
    const std::string& api_name, IPC::Message* reply_msg) {
  CHECK(delegate_);
  delegate_->OnCheckAPIAccessControl(render_process_host_->GetID(),
                                     extension_name, api_name,
      base::Bind(&XWalkExtensionProcessHost::ReplyAccessControlToExtension,
                 base::Unretained(this),
                 reply_msg));
}

void XWalkExtensionProcessHost::OnRegisterPermissions(
    const std::string& extension_name,
    const std::string& perm_table, bool* result) {
  CHECK(delegate_);
  *result = delegate_->OnRegisterPermissions(
      render_process_host_->GetID(), extension_name, perm_table);
}

bool XWalkExtensionProcessHost::Send(IPC::Message* msg) {
  if (process_)
    return process_->GetHost()->Send(msg);
  if (channel_)
    return channel_->Send(msg);
  return false;
}

}  // namespace extensions
}  // namespace xwalk

