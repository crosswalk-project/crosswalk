// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/renderer/binding_channel_host.h"

#include "content/common/child_process.h"
#include "content/common/npobject_base.h"
#include "content/common/npobject_util.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebBindings.h"
#include "v8/include/v8.h"
#include "xwalk/binding/common/binding_messages.h"

namespace xwalk {

// A simple MessageFilter that will ignore all messages and respond to sync
// messages with an error when is_listening_ is false.
class IsListeningFilter : public IPC::ChannelProxy::MessageFilter {
 public:
  IsListeningFilter() : channel_(NULL) {}

  // MessageFilter overrides
  virtual void OnFilterRemoved() OVERRIDE {}
  virtual void OnFilterAdded(IPC::Channel* channel) OVERRIDE {
    channel_ = channel;
  }
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  static bool is_listening_;

 protected:
  virtual ~IsListeningFilter() {}

 private:
  IPC::Channel* channel_;

  DISALLOW_COPY_AND_ASSIGN(IsListeningFilter);
};

bool IsListeningFilter::OnMessageReceived(const IPC::Message& message) {
  if (IsListeningFilter::is_listening_) {
    // Proceed with normal operation.
    return false;
  }

  // Always process message reply to prevent renderer from hanging on sync
  // messages.
  if (message.is_reply() || message.is_reply_error()) {
    return false;
  }

  // Reply to synchronous messages with an error (so they don't block while
  // we're not listening).
  if (message.is_sync()) {
    IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
    reply->set_reply_error();
    channel_->Send(reply);
  }
  return true;
}

// static
bool IsListeningFilter::is_listening_ = true;

// static
bool BindingChannelHost::IsListening() {
  return IsListeningFilter::is_listening_;
}

// static
void BindingChannelHost::SetListening(bool flag) {
  IsListeningFilter::is_listening_ = flag;
}

// static
BindingChannelHost* BindingChannelHost::GetBindingChannelHost(
    const IPC::ChannelHandle& channel_handle,
    base::MessageLoopProxy* ipc_message_loop) {
  BindingChannelHost* result =
      static_cast<BindingChannelHost*>(content::NPChannelBase::GetChannel(
          channel_handle,
          IPC::Channel::MODE_CLIENT,
          ClassFactory,
          ipc_message_loop,
          true,
          content::ChildProcess::current()->GetShutDownEvent()));
  return result;
}

BindingChannelHost::BindingChannelHost() : expecting_shutdown_(false) {
}

BindingChannelHost::~BindingChannelHost() {
}

bool BindingChannelHost::Init(base::MessageLoopProxy* ipc_message_loop,
                              bool create_pipe_now,
                              base::WaitableEvent* shutdown_event) {
  bool ret = content::NPChannelBase::Init(ipc_message_loop,
                                          create_pipe_now, shutdown_event);
  if (ret) {
    is_listening_filter_ = new IsListeningFilter;
    channel_->AddFilter(is_listening_filter_);
  }
  return ret;
}

int BindingChannelHost::GenerateRouteID() {
  int route_id = MSG_ROUTING_NONE;
  Send(new BindingMsg_GenerateRouteID(&route_id));
  return route_id;
}

bool BindingChannelHost::OnControlMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BindingChannelHost, message)
    IPC_MESSAGE_HANDLER(BindingHostMsg_SetException, OnSetException)
    IPC_MESSAGE_HANDLER(BindingHostMsg_ShuttingDown, OnShuttingDown)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  DCHECK(handled);
  return handled;
}

void BindingChannelHost::OnSetException(
    int route_id,
    const content::NPVariant_Param& param) {
  NPVariant value;
  if (!content::CreateNPVariant(param, this, &value, 0, GURL()))
    return;
  content::NPObjectBase* npobj = GetNPObjectListenerForRoute(route_id);

  if (NPVARIANT_IS_STRING(value)) {
    const char* msg = NPVARIANT_TO_STRING(value).UTF8Characters;
    WebKit::WebBindings::setException(
        npobj? npobj->GetUnderlyingNPObject(): NULL, msg);
  } else {
    v8::Handle<v8::Value> ex = WebKit::WebBindings::toV8Value(&value);
    v8::ThrowException(ex);
  }
  WebKit::WebBindings::releaseVariantValue(&value);
}

void BindingChannelHost::OnShuttingDown() {
  expecting_shutdown_ = true;
}

}  // namespace xwalk
