// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_server.h"

#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/memory/shared_memory.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "base/stl_util.h"
#include "content/public/browser/render_process_host.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_sender.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_external_extension.h"

namespace xwalk {
namespace extensions {

// Threshold to determine using shared memory or message
const size_t kInlineMessageMaxSize = 256 * 1024;

XWalkExtensionServer::XWalkExtensionServer()
    : channel_proxy_(NULL),
      permissions_delegate_(NULL) {}

XWalkExtensionServer::~XWalkExtensionServer() {
  DeleteInstanceMap();
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
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_GetExtensions,
        OnGetExtensions)
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

  XWalkExtensionInstance* instance = it->second->CreateInstance();
  if (!instance) {
    LOG(WARNING) << "Can't create instance of extension: " << name
        << ". CreateInstance() return invalid pointer.";
    return;
  }

  instance->SetPostMessageCallback(
      base::Bind(&XWalkExtensionServer::PostMessageToJSCallback,
                 base::Unretained(this), instance_id));

  instance->SetSendSyncReplyCallback(
      base::Bind(&XWalkExtensionServer::SendSyncReplyToJSCallback,
                 base::Unretained(this), instance_id));

  InstanceExecutionData data;
  data.instance = instance;
  data.pending_reply = NULL;

  instances_[instance_id] = data;
}

void XWalkExtensionServer::OnPostMessageToNative(int64_t instance_id,
    const base::ListValue& msg) {
  InstanceMap::const_iterator it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOG(WARNING) << "Can't PostMessage to invalid Extension instance id: "
                 << instance_id;
    return;
  }

  const InstanceExecutionData& data = it->second;

  // The const_cast is needed to remove the only Value contained by the
  // ListValue (which is solely used as wrapper, since Value doesn't
  // have param traits for serialization) and we pass the ownership to to
  // HandleMessage. It is safe to do this because the |msg| won't be used
  // anywhere else when this function returns. Saves a DeepCopy(), which
  // can be costly depending on the size of Value.
  scoped_ptr<base::Value> value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);
  data.instance->HandleMessage(std::move(value));
}

void XWalkExtensionServer::Initialize(IPC::ChannelProxy* channelProxy) {
  base::AutoLock l(channel_proxy_lock_);
  DCHECK(!channel_proxy_);
  channel_proxy_ = channelProxy;
}

bool XWalkExtensionServer::Send(IPC::Message* msg) {
  base::AutoLock l(channel_proxy_lock_);
  if (!channel_proxy_)
    return false;
  return channel_proxy_->Send(msg);
}

namespace {

bool ValidateExtensionIdentifier(const std::string& name) {
  bool dot_allowed = false;
  bool digit_or_underscore_allowed = false;
  for (size_t i = 0; i < name.size(); ++i) {
    char c = name[i];
    if (base::IsAsciiDigit(c)) {
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
    } else if (base::IsAsciiAlpha(c)) {
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
  if (!ValidateExtensionIdentifier(extension->name())) {
    LOG(WARNING) << "Ignoring extension with invalid name: "
                 << extension->name();
    return false;
  }

  if (ContainsKey(extension_symbols_, extension->name())) {
    LOG(WARNING) << "Ignoring extension with name already registered: "
                 << extension->name();
    return false;
  }

  if (!ValidateExtensionEntryPoints(extension->entry_points())) {
    LOG(WARNING) << "Ignoring extension '" << extension->name()
                 << "' with invalid entry point.";
    return false;
  }

  const std::vector<std::string>& entry_points = extension->entry_points();

  for (const std::string& entry_point : entry_points) {
    extension_symbols_.insert(entry_point);
  }

  std::string name = extension->name();
  extension_symbols_.insert(name);
  extensions_[name] = extension.release();
  return true;
}

bool XWalkExtensionServer::ContainsExtension(
    const std::string& extension_name) const {
  return ContainsKey(extensions_, extension_name);
}

void XWalkExtensionServer::PostMessageToJSCallback(
    int64_t instance_id, scoped_ptr<base::Value> msg) {
  base::ListValue wrapped_msg;
  wrapped_msg.Append(msg.release());

  scoped_ptr<IPC::Message> message(
      new XWalkExtensionClientMsg_PostMessageToJS(instance_id, wrapped_msg));
  if (message->size() <= kInlineMessageMaxSize) {
    Send(message.release());
    return;
  }

  base::SharedMemoryCreateOptions options;
  options.size = message->size();
  options.share_read_only = true;

  base::SharedMemory shared_memory;
  if (!shared_memory.Create(options) || !shared_memory.Map(message->size())) {
    LOG(WARNING) << "Can't create shared memory to send out of line message";
    return;
  }

  memcpy(shared_memory.memory(), message->data(), message->size());

  base::SharedMemoryHandle handle;
  base::Process process =
      base::Process::OpenWithExtraPrivileges(channel_proxy_->GetPeerPID());
  CHECK(process.IsValid());
  if (!shared_memory.GiveReadOnlyToProcess(process.Handle(), &handle)) {
    LOG(WARNING) << "Can't share memory handle to send out of line message";
    return;
  }

  Send(new XWalkExtensionClientMsg_PostOutOfLineMessageToJS(handle,
                                                            message->size()));
}

void XWalkExtensionServer::SendSyncReplyToJSCallback(
    int64_t instance_id, scoped_ptr<base::Value> reply) {

  InstanceMap::iterator it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOG(WARNING) << "Can't SendSyncMessage to invalid Extension instance id: "
                 << instance_id;
    return;
  }

  InstanceExecutionData& data = it->second;
  if (!data.pending_reply) {
    LOG(WARNING) << "There's no pending SyncMessage for instance id: "
                 << instance_id;
    return;
  }

  base::ListValue wrapped_reply;
  wrapped_reply.Append(reply.release());
  XWalkExtensionServerMsg_SendSyncMessageToNative::WriteReplyParams(
      data.pending_reply, wrapped_reply);
  Send(data.pending_reply);

  data.pending_reply = NULL;
}

void XWalkExtensionServer::DeleteInstanceMap() {
  InstanceMap::iterator it = instances_.begin();
  int pending_replies_left = 0;

  for (; it != instances_.end(); ++it) {
    delete it->second.instance;
    if (it->second.pending_reply) {
      pending_replies_left++;
      delete it->second.pending_reply;
    }
  }

  instances_.clear();

  if (pending_replies_left > 0) {
    LOG(WARNING) << pending_replies_left
                 << " pending replies left when destroying server.";
  }
}

bool XWalkExtensionServer::ValidateExtensionEntryPoints(
    const std::vector<std::string>& entry_points) {
  for (const std::string& entry_point : entry_points) {
    if (!ValidateExtensionIdentifier(entry_point))
      return false;

    if (ContainsKey(extension_symbols_, entry_point)) {
      LOG(WARNING) << "Entry point '" << entry_point
                   << "' clashes with another extension entry point.";
      return false;
    }
  }

  return true;
}

void XWalkExtensionServer::OnSendSyncMessageToNative(int64_t instance_id,
    const base::ListValue& msg, IPC::Message* ipc_reply) {
  InstanceMap::iterator it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOG(WARNING) << "Can't SendSyncMessage to invalid Extension instance id: "
                 << instance_id;
    return;
  }

  InstanceExecutionData& data = it->second;
  if (data.pending_reply) {
    LOG(WARNING) << "There's already a pending Sync Message for "
                 << "Extension instance id: " << instance_id;
    return;
  }

  data.pending_reply = ipc_reply;

  // The const_cast is needed to remove the only Value contained by the
  // ListValue (which is solely used as wrapper, since Value doesn't
  // have param traits for serialization) and we pass the ownership to to
  // HandleMessage. It is safe to do this because the |msg| won't be used
  // anywhere else when this function returns. Saves a DeepCopy(), which
  // can be costly depending on the size of Value.
  scoped_ptr<base::Value> value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);
  XWalkExtensionInstance* instance = data.instance;

  instance->HandleSyncMessage(std::move(value));
}

void XWalkExtensionServer::OnDestroyInstance(int64_t instance_id) {
  InstanceMap::iterator it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOG(WARNING) << "Can't destroy inexistent instance:" << instance_id;
    return;
  }

  InstanceExecutionData& data = it->second;

  delete data.instance;
  instances_.erase(it);

  Send(new XWalkExtensionClientMsg_InstanceDestroyed(instance_id));
}

void XWalkExtensionServer::OnGetExtensions(
    std::vector<XWalkExtensionServerMsg_ExtensionRegisterParams>* reply) {
  ExtensionMap::iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it) {
    XWalkExtensionServerMsg_ExtensionRegisterParams extension_parameters;
    XWalkExtension* extension = it->second;

    extension_parameters.name = extension->name();
    extension_parameters.js_api = extension->javascript_api();

    const std::vector<std::string>& entry_points = extension->entry_points();
    for (const std::string& entry_point : entry_points) {
      extension_parameters.entry_points.push_back(entry_point);
    }

    reply->push_back(extension_parameters);
  }
}

void XWalkExtensionServer::Invalidate() {
  base::AutoLock l(channel_proxy_lock_);
  channel_proxy_ = NULL;
}

namespace {
base::FilePath::StringType GetNativeLibraryPattern() {
  const base::string16 library_pattern = base::GetNativeLibraryName(
      base::UTF8ToUTF16("*"));
#if defined(OS_WIN)
  return library_pattern;
#else
  return base::UTF16ToUTF8(library_pattern);
#endif
}
}  // namespace

std::vector<std::string> RegisterExternalExtensionsInDirectory(
    XWalkExtensionServer* server, const base::FilePath& dir,
    scoped_ptr<base::ValueMap> runtime_variables) {
  CHECK(server);

  std::vector<std::string> registered_extensions;

  if (!base::DirectoryExists(dir)) {
    LOG(WARNING) << "Couldn't load external extensions from non-existent"
                 << " directory " << dir.AsUTF8Unsafe();
    return registered_extensions;
  }

  base::FileEnumerator libraries(
      dir, false, base::FileEnumerator::FILES, GetNativeLibraryPattern());

  for (base::FilePath extension_path = libraries.Next();
        !extension_path.empty(); extension_path = libraries.Next()) {
    scoped_ptr<XWalkExternalExtension> extension(
        new XWalkExternalExtension(extension_path));

    // Let the extension know about its own path, so it can be used
    // as an identifier in case you have symlinks to extensions to force it
    // load multiple times.
    (*runtime_variables)["extension_path"] =
        new base::StringValue(extension_path.AsUTF8Unsafe());

    extension->set_runtime_variables(*runtime_variables);
    if (server->permissions_delegate())
      extension->set_permissions_delegate(server->permissions_delegate());
    if (extension->Initialize()) {
      registered_extensions.push_back(extension->name());
      server->RegisterExtension(std::move(extension));
    } else {
      LOG(WARNING) << "Failed to initialize extension: "
                   << extension_path.AsUTF8Unsafe();
    }
  }

  return registered_extensions;
}

bool ValidateExtensionNameForTesting(const std::string& extension_name) {
  return ValidateExtensionIdentifier(extension_name);
}

}  // namespace extensions
}  // namespace xwalk

