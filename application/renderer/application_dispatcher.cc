// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/renderer/application_dispatcher.h"

#include <string>

#include "base/logging.h"
#include "content/public/renderer/render_thread.h"
#include "xwalk/application/common/application_messages.h"

namespace xwalk {
namespace application {

ApplicationDispatcher::ApplicationDispatcher() {
  content::RenderThread* thread = content::RenderThread::Get();
  thread->AddObserver(this);
}

ApplicationDispatcher::~ApplicationDispatcher() {
  content::RenderThread::Get()->RemoveObserver(this);
}

bool ApplicationDispatcher::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ApplicationDispatcher, message)
    IPC_MESSAGE_HANDLER(ApplicationMsg_ShouldSuspend, OnShouldSuspend)
    IPC_MESSAGE_HANDLER(ApplicationMsg_Suspend, OnSuspend)
    IPC_MESSAGE_HANDLER(ApplicationMsg_CancelSuspend, OnCancelSuspend)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void ApplicationDispatcher::OnShouldSuspend(int sequence_id) {
  content::RenderThread::Get()->Send(
      new ApplicationHostMsg_ShouldSuspendAck(sequence_id));
}

void ApplicationDispatcher::OnSuspend() {
  content::RenderThread::Get()->Send(new ApplicationHostMsg_SuspendAck);
}

void ApplicationDispatcher::OnCancelSuspend() {
  // NOTIMPLEMENTED();
}

}  // namespace application
}  // namespace xwalk
