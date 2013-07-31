// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/binding/binding_channel.h"

#include "base/bind.h"
#include "base/process_util.h"
#include "base/stringprintf.h"
#include "content/common/child_process.h"
#include "content/common/npobject_proxy.h"
#include "xwalk/binding/binding/binding_service.h"
#include "xwalk/binding/common/binding_messages.h"

namespace xwalk {

namespace {

void BindingReleaseCallback() {
  content::ChildProcess::current()->ReleaseProcess();
}

// How long we wait before releasing the binding process.
const int kBindingReleaseTimeMinutes = 5;

}  // namespace

BindingChannel* BindingChannel::GetBindingChannel(
    int renderer_id, base::MessageLoopProxy* ipc_message_loop) {
  // Map renderer ID to a (single) channel to that process.
  std::string channel_key = base::StringPrintf(
      "%d.r%d", base::GetCurrentProcId(), renderer_id);

  BindingChannel* channel =
      static_cast<BindingChannel*>(content::NPChannelBase::GetChannel(
          channel_key,
          IPC::Channel::MODE_SERVER,
          ClassFactory,
          ipc_message_loop,
          false,
          content::ChildProcess::current()->GetShutDownEvent()));

  if (channel) {
    channel->renderer_id_ = renderer_id;
  }

  return channel;
}

// static
void BindingChannel::NotifyRenderersOfPendingShutdown() {
  Broadcast(new BindingHostMsg_ShuttingDown());
}

int BindingChannel::GenerateRouteID() {
  static int last_id = 1;  // 0 is reserved for window object
  return ++last_id;
}

BindingChannel::~BindingChannel() {
  base::MessageLoop::current()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&BindingReleaseCallback),
      base::TimeDelta::FromMinutes(kBindingReleaseTimeMinutes));
}

BindingChannel::BindingChannel()
    : renderer_id_(-1),
      window_npobject_(NULL) {
  set_send_unblocking_only_during_unblock_dispatch();
  content::ChildProcess::current()->AddRefProcess();
}

void BindingChannel::SetRootObject(const GURL& url) {
  if (!window_npobject_) {
    window_npobject_ =
        content::NPObjectProxy::Create(this, 0, renderer_id_, url);
  }
}

bool BindingChannel::OnControlMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BindingChannel, msg)
    IPC_MESSAGE_HANDLER(BindingMsg_GenerateRouteID, OnGenerateRouteID)
    IPC_MESSAGE_HANDLER(BindingMsg_BindAPIs, OnBindAPIs)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  DCHECK(handled);
  return handled;
}

void BindingChannel::OnGenerateRouteID(int* route_id) {
  *route_id = GenerateRouteID();
}

void BindingChannel::OnBindAPIs(bool* result) {
  // Call BindingService to bind APIs
  BindingService* service = BindingService::GetService();
  for (size_t i = 0; i < features_.size(); i++) {
    service->BindFeature(window_npobject_, features_[i]);
  }

  *result = true;
}

}  // namespace xwalk
