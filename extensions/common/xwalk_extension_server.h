// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_

#include "ipc/ipc_listener.h"
#include "ipc/ipc_sender.h"

namespace xwalk {
namespace extensions {

// This class holds the Native context of Extensions. It can live in the Browser
// Process (for in-process extensions) or on the Extension Process. It
// communicates with its associated XWalkExtensionClient through an IPC channel.
class XWalkExtensionServer : public IPC::Listener, public IPC::Sender {
 public:
  XWalkExtensionServer() {}
  virtual ~XWalkExtensionServer() {}

  // IPC::Listener Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) { return true; }

  // IPC::Sender Implementation.
  virtual bool Send(IPC::Message* msg) { return true; }
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
