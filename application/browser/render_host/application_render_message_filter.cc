// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/render_host/application_render_message_filter.h"

#include "base/bind.h"
#include "chrome/browser/browser_process.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/common/application_messages.h"

using content::BrowserThread;

namespace xwalk {
namespace application {

ApplicationRenderMessageFilter::ApplicationRenderMessageFilter(
    int render_process_id,
    ApplicationSystem* application_sytem)
    : render_process_id_(render_process_id),
      application_system_(application_sytem),
      weak_ptr_factory_(this) {
}

ApplicationRenderMessageFilter::~ApplicationRenderMessageFilter() {
}

bool ApplicationRenderMessageFilter::OnMessageReceived(
    const IPC::Message& message,
    bool* message_was_ok) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP_EX(ApplicationRenderMessageFilter,
                           message,
                           *message_was_ok)
    IPC_MESSAGE_HANDLER(ApplicationHostMsg_ShouldSuspendAck,
                        OnApplicationShouldSuspendAck)
    IPC_MESSAGE_HANDLER(ApplicationHostMsg_SuspendAck,
                        OnApplicationSuspendAck)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void ApplicationRenderMessageFilter::OverrideThreadForMessage(
    const IPC::Message& message, BrowserThread::ID* thread) {
  switch (message.type()) {
    case ApplicationHostMsg_ShouldSuspendAck::ID:
    case ApplicationHostMsg_SuspendAck::ID:
      *thread = BrowserThread::UI;
      break;
    default:
      break;
  }
}

void ApplicationRenderMessageFilter::OnApplicationShouldSuspendAck(
         int sequence_id) {
  if (application_system_->process_manager()) {
    application_system_->process_manager()->OnShouldSuspendAck(sequence_id);
  }
}

void ApplicationRenderMessageFilter::OnApplicationSuspendAck() {
  if (application_system_->process_manager()) {
    application_system_->process_manager()->OnSuspendAck();
  }
}

}  // namespace xwalk
}  // namespace application
