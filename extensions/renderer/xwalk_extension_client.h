// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_

#include <stdint.h>
#include <map>
#include <string>

#include "base/memory/scoped_ptr.h"
#include "ipc/ipc_listener.h"
#include "xwalk/extensions/renderer/xwalk_remote_extension_runner.h"

namespace base {
class ListValue;
}

namespace IPC {
class Sender;
}

namespace xwalk {
namespace extensions {

class XWalkModuleSystem;

// This class holds the JavaScript context of Extensions. It lives in the
// Render Process and communicates directly with its associated
// XWalkExtensionServer through an IPC channel.
class XWalkExtensionClient : public IPC::Listener {
 public:
  explicit XWalkExtensionClient();
  virtual ~XWalkExtensionClient();

  // IPC::Listener Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void CreateRunnersForModuleSystem(XWalkModuleSystem* module_system);

  void DestroyInstance(int64_t instance_id);

  void PostMessageToNative(int64_t instance_id, scoped_ptr<base::Value> msg);
  scoped_ptr<base::Value> SendSyncMessageToNative(int64_t instance_id,
      scoped_ptr<base::Value> msg);

  void Initialize(IPC::Sender* sender) { sender_ = sender; }

 private:
  XWalkRemoteExtensionRunner* CreateRunner(const std::string& extension_name,
      XWalkRemoteExtensionRunner::Client* client);

  bool Send(IPC::Message* msg);

  // Message Handlers.
  void OnInstanceDestroyed(int64_t instance_id);
  void OnPostMessageToJS(int64_t instance_id, const base::ListValue& msg);
  void OnRegisterExtension(const std::string& name, const std::string& api) {
    extension_apis_[name] = api;
  }

  IPC::Sender* sender_;

  typedef std::map<std::string, std::string> ExtensionAPIMap;
  ExtensionAPIMap extension_apis_;

  typedef std::map<int64_t, XWalkRemoteExtensionRunner*> RunnerMap;
  RunnerMap runners_;

  int64_t next_instance_id_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_
