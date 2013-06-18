// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/extensions/browser/cameo_extension_host.h"

#include "cameo/src/extensions/common/cameo_extension_messages.h"
#include <string>

using content::BrowserThread;

namespace cameo {
namespace extensions {

CameoExtensionHost::CameoExtensionHost() : channel_(NULL) {}

CameoExtensionHost::~CameoExtensionHost() {
  ExtensionMap::iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it)
    delete it->second.extension;
}

static BrowserThread::ID BrowserThreadFromHandlerThread(
    CameoExtension::HandlerThread thread) {
  switch (thread) {
    case CameoExtension::HANDLER_THREAD_FILE:
      return BrowserThread::FILE;
    case CameoExtension::HANDLER_THREAD_UI:
      return BrowserThread::UI;
  }
  NOTREACHED();
  return BrowserThread::FILE;
}

bool CameoExtensionHost::RegisterExtension(CameoExtension* extension) {
  std::string name = extension->name();
  if (extensions_.find(name) != extensions_.end())
    return false;

  ExtensionEntry entry;
  entry.extension = extension;
  entry.thread = BrowserThreadFromHandlerThread(extension->thread());
  extensions_[name] = entry;
  Send(new CameoViewMsg_RegisterExtension(name, extension->GetJavaScriptAPI()));
  return true;
}

void CameoExtensionHost::OnFilterAdded(IPC::Channel* channel) {
  channel_ = channel;
}

bool CameoExtensionHost::OnMessageReceived(const IPC::Message& message) {
  CHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(CameoExtensionHost, message)
    IPC_MESSAGE_HANDLER_GENERIC(CameoViewHostMsg_PostMessage,
                                OnPostMessage(message))
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

bool CameoExtensionHost::Send(IPC::Message* message) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(
        BrowserThread::IO,
        FROM_HERE,
        base::Bind(base::IgnoreResult(&CameoExtensionHost::Send), this,
                   message));
    return true;
  }

  if (channel_)
    return channel_->Send(message);

  delete message;
  return false;
}

void CameoExtensionHost::PostMessage(const int32_t render_view_id,
                                     const std::string& extension,
                                     const std::string& msg) {
  // FIXME(cmarcelo): Can we check the validity of id?
  Send(new CameoViewMsg_PostMessage(render_view_id, extension, msg));
}

void CameoExtensionHost::OnPostMessage(const IPC::Message& message) {
  CameoViewHostMsg_PostMessage::Param param;
  if (!CameoViewHostMsg_PostMessage::Read(&message, &param))
    return;
  const std::string& extension = param.a;
  const std::string& msg = param.b;
  ExtensionMap::iterator it = extensions_.find(extension);
  if (it != extensions_.end()) {
    CameoExtension* cameo_extension = it->second.extension;
    BrowserThread::PostTask(
        it->second.thread,
        FROM_HERE,
        base::Bind(&CameoExtension::HandleMessage,
                   base::Unretained(cameo_extension),
                   message.routing_id(), msg));
  }
}

}  // namespace extensions
}  // namespace cameo
