// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_RUNNER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_RUNNER_H_

#include <string>
#include "base/basictypes.h"

namespace xwalk {
namespace extensions {

// This interface is used to interact with an extension context. Using an
// interface allow us to implement different strategies: running the context
// directly, in a thread environment or even in a separated process.
//
// Subclasses of runner should implement HandleMessageFromClient() and also call
// PostMessageToClient when appropriate. See the concrete subclasses for
// examples.
//
// To use a context runner, the object should implement its Client interface to
// receive messages from context, and call PostMessageToContext() when
// appropriate.
class XWalkExtensionRunner {
 public:
  class Client {
   public:
    virtual void HandleMessageFromContext(
        const XWalkExtensionRunner* runner, const std::string& msg) = 0;
   protected:
    virtual ~Client() {}
  };

  XWalkExtensionRunner(const std::string& extension_name, Client* client);
  virtual ~XWalkExtensionRunner();

  void PostMessageToContext(const std::string& msg);

  std::string extension_name() const { return extension_name_; }

 protected:
  void PostMessageToClient(const std::string& msg);
  virtual void HandleMessageFromClient(const std::string& msg) = 0;

  Client* client_;

 private:
  std::string extension_name_;
  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionRunner);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_RUNNER_H_
