// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_

#include "ipc/ipc_listener.h"
#include "ipc/ipc_sender.h"

namespace xwalk {
namespace extensions {

// This class holds the JavaScript context of Extensions. It lives in the
// Render Process and communicates directly with its associated
// XWalkExtensionServer through an IPC channel.
class XWalkExtensionClient : public IPC::Listener, public IPC::Sender {
 public:
  XWalkExtensionClient() {}
  virtual ~XWalkExtensionClient() {}

  // IPC::Listener Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) { return true; }

  // IPC::Sender Implementation.
  virtual bool Send(IPC::Message* msg) { return true; }
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_
