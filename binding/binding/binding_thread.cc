// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/binding/binding_thread.h"

#include "build/build_config.h"
#include "content/common/child_process.h"
#include "content/common/npobject_util.h"
#include "content/public/browser/browser_thread.h"
#include "xwalk/binding/binding/binding_channel.h"
#include "xwalk/binding/binding/binding_service.h"
#include "xwalk/binding/common/binding_messages.h"

#if defined(OS_POSIX)
#include "base/file_descriptor_posix.h"
#endif

namespace xwalk {

BindingThread::BindingThread()
    : in_process_(false) {
  Init();
}

BindingThread::BindingThread(const std::string& channel_id)
    : ChildThread(channel_id) {
  in_process_ = !content::ChildProcess::current();
  Init();
}

BindingThread::~BindingThread() {
}

void BindingThread::Init() {
  content::PatchNPNFunctions();

  message_loop()->set_exception_restoration(true);
  BindingService::GetService();
}

void BindingThread::Shutdown() {
  content::NPChannelBase::CleanupChannels();
}

bool BindingThread::OnControlMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BindingThread, msg)
    IPC_MESSAGE_HANDLER(BindingProcessMsg_CreateChannel, OnCreateChannel)
    IPC_MESSAGE_HANDLER(BindingProcessMsg_ShuttingDown, OnShuttingDown)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void BindingThread::OnCreateChannel(
    int renderer_id,
    const GURL& url,
    const std::vector<std::string>& features) {
  // Create API channel
  scoped_refptr<BindingChannel> channel;
  if (in_process_) {
    channel = BindingChannel::GetBindingChannel(renderer_id,
        content::BrowserThread::GetMessageLoopProxyForThread(
            content::BrowserThread::IO));
  } else {
    channel = BindingChannel::GetBindingChannel(renderer_id,
        content::ChildProcess::current()->io_message_loop_proxy());
  }
  channel->SetFeatures(features);

  // return API channel handle
  IPC::ChannelHandle channel_handle;
  if (channel.get()) {
    channel_handle.name = channel->channel_handle().name;
#if defined(OS_POSIX)
    channel_handle.socket =
        base::FileDescriptor(channel->TakeRendererFileDescriptor(), true);
#endif
    channel->SetRootObject(url);
  }

  Send(new BindingProcessHostMsg_ChannelCreated(renderer_id, channel_handle));
}

void BindingThread::OnShuttingDown() {
  BindingChannel::NotifyRenderersOfPendingShutdown();
}

}  // namespace xwalk
