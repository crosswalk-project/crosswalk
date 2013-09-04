// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_message_filter.h"

#include "content/public/browser/browser_thread.h"
#include "ipc/ipc_message_utils.h"
#include "xwalk/extensions/browser/xwalk_extension_runner_store.h"
#include "xwalk/extensions/browser/xwalk_extension_web_contents_handler.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace xwalk {
namespace extensions {

XWalkExtensionMessageFilter::XWalkExtensionMessageFilter(
    XWalkExtensionWebContentsHandler* handler)
    : handler_(handler),
      runners_(handler->runner_store()),
      routing_id_(handler->routing_id()),
      is_valid_(true) {
}

XWalkExtensionMessageFilter::~XWalkExtensionMessageFilter() {}

void XWalkExtensionMessageFilter::PostMessage(
    const XWalkExtensionRunner* runner, scoped_ptr<base::Value> msg) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

  base::AutoLock l(is_valid_lock_);
  if (!is_valid_)
    return;

  base::ListValue list;
  list.Append(msg.release());

  int64_t frame_id = runners_->GetFrameForRunner(runner);
  Send(new XWalkViewMsg_PostMessageToJS(routing_id_,
                                    frame_id,
                                    runner->extension_name(),
                                    list));
}

void XWalkExtensionMessageFilter::PostReplyMessage(
    scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) {
  base::AutoLock l(is_valid_lock_);
  if (!is_valid_)
    return;

  base::ListValue result;
  result.Append(msg.release());

  IPC::WriteParam(ipc_reply.get(), result);
  Send(ipc_reply.release());
}

bool XWalkExtensionMessageFilter::OnMessageReceived(
    const IPC::Message& message, bool* message_was_ok) {
  // The filter gets called for every single message that arrives
  // in the IO Thread, but we are only interest on the ones routed
  // to WebContents associated to this filter via a WebContentsHandler.
  if (message.routing_id() != routing_id_)
    return false;

  base::AutoLock l(is_valid_lock_);
  if (!is_valid_)
    return false;

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionMessageFilter, message)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_PostMessageToNative, OnPostMessage)
    IPC_MESSAGE_HANDLER_DELAY_REPLY(XWalkViewHostMsg_SendSyncMessage,
                                    OnSendSyncMessage)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_DidCreateScriptContext,
                        DidCreateScriptContext)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_WillReleaseScriptContext,
                        WillReleaseScriptContext)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkExtensionMessageFilter::OnPostMessage(
    int64_t frame_id, const std::string& extension_name,
    const base::ListValue& msg) {
  XWalkExtensionRunner* runner =
      runners_->GetRunnerByFrameAndName(frame_id, extension_name);
  if (!runner) {
    LOG(WARNING) << "Couldn't handle message for unregistered extension '"
                 << extension_name << "'";
    return;
  }

  // The const_cast is needed to remove the only Value contained by the
  // ListValue (which is solely used as wrapper, since Value doesn't
  // have param traits for serialization) and we pass the ownership to to
  // HandleMessage. It is safe to do this because the |msg| won't be used
  // anywhere else when this function returns. Saves a DeepCopy(), which
  // can be costly depending on the size of Value.
  base::Value* value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);
  runner->PostMessageToNative(scoped_ptr<base::Value>(value));
}

void XWalkExtensionMessageFilter::OnSendSyncMessage(
    int64_t frame_id, const std::string& extension_name,
    const base::ListValue& msg, IPC::Message* ipc_reply) {
  XWalkExtensionRunner* runner =
      runners_->GetRunnerByFrameAndName(frame_id, extension_name);
  if (!runner) {
    LOG(WARNING) << "Couldn't handle message for unregistered extension '"
                 << extension_name << "'";
    return;
  }

  base::Value* value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);

  // We handle a pre-populated |ipc_reply| to the Context, so it is up to the
  // Context to decide when to reply. It is important to notice that the callee
  // on the renderer will remain blocked until the reply gets back.
  runner->SendSyncMessageToNative(scoped_ptr<IPC::Message>(ipc_reply),
                                   scoped_ptr<base::Value>(value));
}

void XWalkExtensionMessageFilter::DidCreateScriptContext(int64_t frame_id) {
  runners_->AddFrame(frame_id);

  // FIXME(tmpsantos): This is going to be called on IO Thread, but the handler
  // lives on the UI Thread. If we try to post the call to the IO Thread, we are
  // at the risk of receiving messages from the renderer and fail because the
  // extension was not registered yet. It is safe to make this call, but the
  // code path should be clarified. The solution would be move the ownership
  // of the RunnerStore to this filter and make sure that
  // ExtensionService::CreateRunnersForHandler is thread safe.
  handler_->DidCreateScriptContext(frame_id);
}

void XWalkExtensionMessageFilter::WillReleaseScriptContext(int64_t frame_id) {
  runners_->DeleteFrame(frame_id);
}

void XWalkExtensionMessageFilter::Invalidate() {
  base::AutoLock l(is_valid_lock_);
  is_valid_ = false;
}

}  // namespace extensions
}  // namespace xwalk
