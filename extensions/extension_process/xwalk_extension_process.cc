// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/extension_process/xwalk_extension_process.h"

#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "ipc/ipc_switches.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_sender.h"
#include "ipc/ipc_sync_channel.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"

namespace xwalk {
namespace extensions {

// FIXME(jeez): Remove this.
class DummySender : public IPC::Sender {
 public:
  virtual bool Send(IPC::Message* msg) OVERRIDE { VLOG(0)
      << "DummySender::Send()"; return true; }

  virtual ~DummySender() {}
};

XWalkExtensionProcess::XWalkExtensionProcess()
    : main_loop_(base::MessageLoop::TYPE_UI),
      shutdown_event_(false, false),
      io_thread_("XWalkExtensionProcess_IOThread") {
  io_thread_.StartWithOptions(
      base::Thread::Options(base::MessageLoop::TYPE_IO, 0));
  extensions_server_.reset(new XWalkExtensionServer());

  // FIXME(jeez): Remove this.
  dummy_sender_.reset(new DummySender());

  CreateChannel();
}

XWalkExtensionProcess::~XWalkExtensionProcess() {
  // FIXME(jeez): Move this to OnChannelClosing/Error/Disconnected when we have
  // our MessageFilter set.
  extensions_server_->Invalidate();

  shutdown_event_.Signal();
  io_thread_.Stop();
}

void XWalkExtensionProcess::Run() {
  run_loop_.Run();
}

bool XWalkExtensionProcess::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionProcess, message)
    IPC_MESSAGE_HANDLER(XWalkExtensionProcessMsg_RegisterExtensions,
                        OnRegisterExtensions)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionProcess::OnRegisterExtensions(
    const base::FilePath& path) {
  VLOG(0) << "\nExtensions to register from: " << path.value();
}

void XWalkExtensionProcess::CreateChannel() {
  std::string channel_id =
      CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          switches::kProcessChannelID);
  browser_process_channel_.reset(new IPC::SyncChannel(channel_id,
      IPC::Channel::MODE_CLIENT, this, io_thread_.message_loop_proxy(),
      true, &shutdown_event_));

  // FIXME(jeez): Change this so we pass our EP IPC Channel.
  extensions_server_->Initialize(dummy_sender_.get());
}

void XWalkExtensionProcess::HandleMessageFromNative(
    const XWalkExtensionRunner* runner, scoped_ptr<base::Value> msg) {
}

void XWalkExtensionProcess::HandleReplyMessageFromNative(
    scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) {
}

}  // namespace extensions
}  // namespace xwalk
