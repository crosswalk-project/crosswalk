// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_server.h"

#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "base/stl_util.h"
#include "content/public/browser/render_process_host.h"
#include "ipc/ipc_sender.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_external_extension.h"

namespace xwalk {
namespace extensions {

XWalkExtensionServer::XWalkExtensionServer()
    : sender_(NULL) {}

XWalkExtensionServer::~XWalkExtensionServer() {
  STLDeleteValues(&instances_);
  STLDeleteValues(&extensions_);
}

bool XWalkExtensionServer::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionServer, message)
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_CreateInstance,
        OnCreateInstance)
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_DestroyInstance,
        OnDestroyInstance)
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_PostMessageToNative,
        OnPostMessageToNative)
    IPC_MESSAGE_HANDLER_DELAY_REPLY(
        XWalkExtensionServerMsg_SendSyncMessageToNative,
        OnSendSyncMessageToNative)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkExtensionServer::OnCreateInstance(int64_t instance_id,
    std::string name) {
  ExtensionMap::const_iterator it = extensions_.find(name);

  if (it == extensions_.end()) {
    LOG(WARNING) << "Can't create instance of extension: " << name
        << ". Extension is not registered.";
    return;
  }

  XWalkExtensionInstance* instance = it->second->CreateInstance(
      base::Bind(&XWalkExtensionServer::PostMessageToJSCallback,
                 base::Unretained(this), instance_id));
  instances_[instance_id] = instance;
}

void XWalkExtensionServer::OnPostMessageToNative(int64_t instance_id,
    const base::ListValue& msg) {
  InstanceMap::const_iterator it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOG(WARNING) << "Can't PostMessage to invalid Extension instance id: "
                 << instance_id;
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
  it->second->HandleMessage(scoped_ptr<base::Value>(value));
}

void XWalkExtensionServer::Initialize(IPC::Sender* sender) {
  base::AutoLock l(sender_lock_);
  DCHECK(!sender_);
  sender_ = sender;
}

bool XWalkExtensionServer::Send(IPC::Message* msg) {
  base::AutoLock l(sender_lock_);
  if (!sender_)
    return false;
  return sender_->Send(msg);
}

namespace {

bool ValidateExtensionName(const std::string& extension_name) {
  bool dot_allowed = false;
  bool digit_or_underscore_allowed = false;
  for (size_t i = 0; i < extension_name.size(); ++i) {
    char c = extension_name[i];
    if (IsAsciiDigit(c)) {
      if (!digit_or_underscore_allowed)
        return false;
    } else if (c == '_') {
      if (!digit_or_underscore_allowed)
        return false;
    } else if (c == '.') {
      if (!dot_allowed)
        return false;
      dot_allowed = false;
      digit_or_underscore_allowed = false;
    } else if (IsAsciiAlpha(c)) {
      dot_allowed = true;
      digit_or_underscore_allowed = true;
    } else {
      return false;
    }
  }

  // If after going through the entire name we finish with dot_allowed, it means
  // the previous character is not a dot, so it's a valid name.
  return dot_allowed;
}

}  // namespace

bool XWalkExtensionServer::RegisterExtension(
    scoped_ptr<XWalkExtension> extension) {
  if (!ValidateExtensionName(extension->name())) {
    LOG(WARNING) << "Ignoring extension with invalid name: "
                 << extension->name();
    return false;
  }

  if (extensions_.find(extension->name()) != extensions_.end()) {
    LOG(WARNING) << "Ignoring extension with name already registered: "
                 << extension->name();
    return false;
  }

  std::string name = extension->name();
  extensions_[name] = extension.release();
  return true;
}

void XWalkExtensionServer::PostMessageToJSCallback(
    int64_t instance_id, scoped_ptr<base::Value> msg) {
  base::ListValue wrapped_msg;
  wrapped_msg.Append(msg.release());
  Send(new XWalkExtensionClientMsg_PostMessageToJS(instance_id, wrapped_msg));
}

void XWalkExtensionServer::OnSendSyncMessageToNative(int64_t instance_id,
    const base::ListValue& msg, IPC::Message* ipc_reply) {
  InstanceMap::const_iterator it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOG(WARNING) << "Can't SendSyncMessage to invalid Extension instance id: "
                 << instance_id;
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
  scoped_ptr<base::Value> reply(
      it->second->HandleSyncMessage(scoped_ptr<base::Value>(value)));

  base::ListValue wrapped_reply;
  wrapped_reply.Append(reply.release());
  IPC::WriteParam(ipc_reply, wrapped_reply);
  Send(ipc_reply);
}

void XWalkExtensionServer::OnDestroyInstance(int64_t instance_id) {
  InstanceMap::iterator it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOG(WARNING) << "Can't destroy inexistent instance:" << instance_id;
    return;
  }

  delete it->second;
  instances_.erase(it);

  Send(new XWalkExtensionClientMsg_InstanceDestroyed(instance_id));
}

void XWalkExtensionServer::RegisterExtensionsInRenderProcess() {
  // Having a sender means we have a RenderProcessHost ready.
  DCHECK(sender_);

  ExtensionMap::iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it) {
    XWalkExtension* extension = it->second;
    Send(new XWalkExtensionClientMsg_RegisterExtension(
        extension->name(), extension->GetJavaScriptAPI()));
  }
}

void XWalkExtensionServer::Invalidate() {
  base::AutoLock l(sender_lock_);
  sender_ = NULL;
}

void XWalkExtensionServer::OnChannelConnected(int32 peer_pid) {
  RegisterExtensionsInRenderProcess();
}

namespace {
base::FilePath::StringType GetNativeLibraryPattern() {
  const base::string16 library_pattern = base::GetNativeLibraryName(
      UTF8ToUTF16("*"));
#if defined(OS_WIN)
  return library_pattern;
#else
  return UTF16ToUTF8(library_pattern);
#endif
}
}  // namespace

void RegisterExternalExtensionsInDirectory(
    XWalkExtensionServer* server, const base::FilePath& dir) {
  CHECK(server);

  if (!file_util::DirectoryExists(dir)) {
    LOG(WARNING) << "Couldn't load external extensions from non-existent"
                 << " directory " << dir.AsUTF8Unsafe();
    return;
  }

  base::FileEnumerator libraries(
      dir, false, base::FileEnumerator::FILES, GetNativeLibraryPattern());

  for (base::FilePath extension_path = libraries.Next();
        !extension_path.empty(); extension_path = libraries.Next()) {
    scoped_ptr<XWalkExternalExtension> extension(
        new XWalkExternalExtension(extension_path));
    if (extension->is_valid())
      server->RegisterExtension(extension.PassAs<XWalkExtension>());
  }
}

bool ValidateExtensionNameForTesting(const std::string& extension_name) {
  return ValidateExtensionName(extension_name);
}

}  // namespace extensions
}  // namespace xwalk

