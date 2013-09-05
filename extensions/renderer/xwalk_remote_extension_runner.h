// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_REMOTE_EXTENSION_RUNNER_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_REMOTE_EXTENSION_RUNNER_H_

#include <stdint.h>
#include <string>
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"

namespace base {
class Value;
}

namespace xwalk {
namespace extensions {

class XWalkExtensionClient;
class XWalkExtensionModule;

// This object provides the exact interface for XWalkExtensionModule, the
// JavaScript binding, to interact with an extension instance. The
// XWalkExtensionModule implements the runner's Client interface to handle
// messages from native. This interface is similar to XWalkExtensionRunner.
//
// TODO(cmarcelo): The interface of this class is conceptually similar to
// XWalkExtensionRunner, consider whether it is worth to make it a
// XWalkExtensionRunner subclass or simply a separated object.
class XWalkRemoteExtensionRunner {
 public:
  class Client {
   public:
    virtual void HandleMessageFromNative(const base::Value& msg) = 0;
   protected:
    virtual ~Client() {}
  };

  XWalkRemoteExtensionRunner(Client* client,
      XWalkExtensionClient* extension_client, int64_t instance_id);
  virtual ~XWalkRemoteExtensionRunner();

  void PostMessageToNative(scoped_ptr<base::Value> msg);
  scoped_ptr<base::Value> SendSyncMessageToNative(
      scoped_ptr<base::Value> msg);

  void PostMessageToJS(const base::Value& msg);

 private:
  friend class XWalkExtensionModule;

  void Destroy();

  Client* client_;
  int64_t instance_id_;
  XWalkExtensionClient* extension_client_;

  DISALLOW_COPY_AND_ASSIGN(XWalkRemoteExtensionRunner);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_REMOTE_EXTENSION_RUNNER_H_
