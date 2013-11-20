// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_

#include <stdint.h>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/synchronization/lock.h"
#include "base/values.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_listener.h"
#include "xwalk/extensions/common/xwalk_extension.h"

struct XWalkExtensionServerMsg_ExtensionRegisterParams;

namespace base {
class FilePath;
}

namespace content {
class RenderProcessHost;
}

namespace IPC {
class Sender;
}

namespace xwalk {
namespace extensions {

class XWalkExtensionInstance;

// Manages the instances for a set of extensions. It communicates with one
// XWalkExtensionClient by means of IPC channel.
//
// This class is used both by in-process extensions running in the Browser
// Process, and by the external extensions running in the Extension Process.
class XWalkExtensionServer : public IPC::Listener {
 public:
  XWalkExtensionServer();
  virtual ~XWalkExtensionServer();

  // IPC::Listener Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void Initialize(IPC::Sender* sender,
      XWalkExtension::PermissionsDelegate* delegate = NULL);
  bool Send(IPC::Message* msg);

  bool RegisterExtension(scoped_ptr<XWalkExtension> extension);
  bool ContainsExtension(const std::string& extension_name) const;

  void Invalidate();

  // These Message Handlers can be accessed by a message filter when
  // running on the browser process.
  void OnCreateInstance(int64_t instance_id, std::string name);
  void OnGetExtensions(
      std::vector<XWalkExtensionServerMsg_ExtensionRegisterParams>* reply);

 private:
  struct InstanceExecutionData {
    XWalkExtensionInstance* instance;
    IPC::Message* pending_reply;
  };

  // Message Handlers
  void OnDestroyInstance(int64_t instance_id);
  void OnPostMessageToNative(int64_t instance_id, const base::ListValue& msg);
  void OnSendSyncMessageToNative(int64_t instance_id,
      const base::ListValue& msg, IPC::Message* ipc_reply);

  void PostMessageToJSCallback(int64_t instance_id,
                               scoped_ptr<base::Value> msg);

  void SendSyncReplyToJSCallback(int64_t instance_id,
                                 scoped_ptr<base::Value> reply);

  void DeleteInstanceMap();

  bool ValidateExtensionEntryPoints(const base::ListValue& entry_points);

  base::Lock sender_lock_;
  IPC::Sender* sender_;

  typedef std::map<std::string, XWalkExtension*> ExtensionMap;
  ExtensionMap extensions_;

  typedef std::map<int64_t, InstanceExecutionData> InstanceMap;
  InstanceMap instances_;

  // The exported symbols for extensions already registered.
  typedef std::set<std::string> ExtensionSymbolsSet;
  ExtensionSymbolsSet extension_symbols_;

  XWalkExtension::PermissionsDelegate* permissions_delegate_;
};

std::vector<std::string> RegisterExternalExtensionsInDirectory(
    XWalkExtensionServer* server, const base::FilePath& dir);

bool ValidateExtensionNameForTesting(const std::string& extension_name);

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
