// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_THREADED_RUNNER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_THREADED_RUNNER_H_

#include <string>
#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"
#include "xwalk/extensions/browser/xwalk_extension.h"

namespace base {
class Thread;
}

namespace tracked_objects {
class Location;
}

namespace xwalk {
namespace extensions {

// Creates and runs an extension context in a thread. All operations
// from the extension context will be called in the separated thread.
class XWalkExtensionThreadedRunner : public XWalkExtensionRunner {
 public:
  XWalkExtensionThreadedRunner(XWalkExtension* extension, Client* client);
  virtual ~XWalkExtensionThreadedRunner();

 private:
  // XWalkExtensionRunner implementation.
  virtual void HandleMessageFromClient(const std::string& msg) OVERRIDE;

  bool CalledOnExtensionThread() const;
  bool PostTaskToExtensionThread(const tracked_objects::Location& from_here,
                                 const base::Closure& task);
  void CreateContext();
  void DestroyContext();

  scoped_ptr<XWalkExtension::Context> context_;
  scoped_ptr<base::Thread> thread_;
  XWalkExtension* extension_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionThreadedRunner);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_THREADED_RUNNER_H_
