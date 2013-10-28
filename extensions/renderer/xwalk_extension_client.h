// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "ipc/ipc_listener.h"

namespace base {
class Value;
}

namespace IPC {
class Sender;
}

namespace xwalk {
namespace extensions {

// This class holds the JavaScript context of Extensions. It lives in the
// Render Process and communicates directly with its associated
// XWalkExtensionServer through an IPC channel.
//
// Users of this class post (and send sync) messages to specific instances and
// are able to handle messages from instances by implementing the
// InstanceHandler interface.
class XWalkExtensionClient : public IPC::Listener {
 public:
  struct InstanceHandler {
    virtual void HandleMessageFromNative(const base::Value& msg) = 0;
   protected:
    ~InstanceHandler() {}
  };

  XWalkExtensionClient();
  virtual ~XWalkExtensionClient();

  int64_t CreateInstance(const std::string& extension_name,
                         InstanceHandler* handler);
  void DestroyInstance(int64_t instance_id);

  void PostMessageToNative(int64_t instance_id, scoped_ptr<base::Value> msg);
  scoped_ptr<base::Value> SendSyncMessageToNative(int64_t instance_id,
      scoped_ptr<base::Value> msg);

  void Initialize(IPC::Sender* sender);

  // IPC::Listener Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  struct ExtensionCodePoints {
    ExtensionCodePoints();
    ~ExtensionCodePoints();
    std::string api;
    std::vector<std::string> entry_points;
  };

  typedef std::map<std::string, ExtensionCodePoints*> ExtensionAPIMap;

  const ExtensionAPIMap& extension_apis() const { return extension_apis_; }

 private:
  bool Send(IPC::Message* msg);

  // Message Handlers.
  void OnInstanceDestroyed(int64_t instance_id);
  void OnPostMessageToJS(int64_t instance_id, const base::ListValue& msg);

  IPC::Sender* sender_;
  ExtensionAPIMap extension_apis_;

  typedef std::map<int64_t, InstanceHandler*> HandlerMap;
  HandlerMap handlers_;

  int64_t next_instance_id_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_
