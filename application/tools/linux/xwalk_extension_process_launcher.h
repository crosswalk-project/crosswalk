// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_XWALK_EXTENSION_PROCESS_LAUNCHER_H_
#define XWALK_APPLICATION_TOOLS_LINUX_XWALK_EXTENSION_PROCESS_LAUNCHER_H_

#include <string>

#include "base/threading/thread.h"

namespace base {
class AtExitManager;
}

namespace xwalk {
namespace extensions {
class XWalkExtensionProcess;
}
}

class XWalkExtensionProcessLauncher: public base::Thread {
 public:
  XWalkExtensionProcessLauncher();
  ~XWalkExtensionProcessLauncher();

  // Implement base::Thread.
  virtual void CleanUp() OVERRIDE;

  // Will be called in launcher's main thread.
  void Launch(const std::string& channel_id, int channel_fd);

  bool is_started() const { return is_started_; }

 private:
  void StartExtensionProcess(const std::string& channel_id, int channel_fd);

  bool is_started_;
  scoped_ptr<base::AtExitManager> exit_manager_;
  scoped_ptr<xwalk::extensions::XWalkExtensionProcess> extension_process_;
};

#endif  // XWALK_APPLICATION_TOOLS_LINUX_XWALK_EXTENSION_PROCESS_LAUNCHER_H_
