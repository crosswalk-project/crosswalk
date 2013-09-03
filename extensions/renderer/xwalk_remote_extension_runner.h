// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_REMOTE_EXTENSION_RUNNER_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_REMOTE_EXTENSION_RUNNER_H_

#include <string>
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"

namespace base {
class Value;
}

namespace xwalk {
namespace extensions {

class XWalkExtensionRenderViewHandler; // FIXME(jeez): Remove this.
class XWalkExtensionClient;
class XWalkExtensionModule;

// This object provides the exact interface for XWalkExtensionModule, the
// JavaScript binding, to interact with an extension instance. The
// XWalkExtensionModule implements the runner's Client interface to handle
// messages from native.
//
// This interface is similar to XWalkExtensionRunner.
//
// TODO(cmarcelo): Is it worth to make the two interfaces the same?
class XWalkRemoteExtensionRunner {
 public:
  class Client {
   public:
    virtual void HandleMessageFromNative(const base::Value& msg) = 0;
   protected:
    virtual ~Client() {}
  };

  // FIXME(jeez): this should be only (Client*, XWalkExtensionClient*, instance_id)
  XWalkRemoteExtensionRunner(XWalkExtensionRenderViewHandler* handler,
      int64_t frame_id, const std::string& extension_name, Client* client,
      XWalkExtensionClient* extension_client, int64_t instance_id);
  virtual ~XWalkRemoteExtensionRunner();

  void PostMessageToNative(scoped_ptr<base::Value> msg);
  scoped_ptr<base::Value> SendSyncMessageToNative(
      scoped_ptr<base::Value> msg);

  void PostMessageToJS(const base::Value& msg);

  std::string extension_name() const { return extension_name_; }

  // FIXME(cmarcelo): Remove once we migrate to using instance ids.
  int64_t frame_id() const { return frame_id_; }
  XWalkExtensionRenderViewHandler* handler() const { return handler_; }

 private:
  friend class XWalkExtensionModule;

  void Destroy();

  Client* client_;
  std::string extension_name_; // FIXME(jeez): Remove this.
  XWalkExtensionRenderViewHandler* handler_; // FIXME(jeez): Remove this.
  int64_t frame_id_; // FIXME(jeez): Remove this.
  int64_t instance_id_;
  XWalkExtensionClient* extension_client_;

  DISALLOW_COPY_AND_ASSIGN(XWalkRemoteExtensionRunner);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_REMOTE_EXTENSION_RUNNER_H_
