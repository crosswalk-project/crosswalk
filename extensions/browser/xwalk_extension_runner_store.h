// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_RUNNER_STORE_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_RUNNER_STORE_H_

#include <stdint.h>
#include <map>
#include <string>
#include "base/basictypes.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionRunner;

// Keeps track of runners for a WebContentsHandler and their associated
// information (name and frame id), providing queries useful for handler
// operation. One WebContentsHandler handles runners for all its frames.
// It lives in the IO Thread, mainly because it is frenquently accessed by
// the ExtensionMessageFilter which lives in the same thread. The Add* and
// Delete* methods can be accessed from any thread and they will make sure
// that the message gets reposted to the IO Thread.
class XWalkExtensionRunnerStore {
 public:
  XWalkExtensionRunnerStore() {}
  ~XWalkExtensionRunnerStore();

  void AddFrame(int64_t frame_id);
  void AddRunner(int64_t frame_id, XWalkExtensionRunner* runner);

  XWalkExtensionRunner* GetRunnerByFrameAndName(int64_t frame_id,
                                                const std::string& name);
  int64_t GetFrameForRunner(const XWalkExtensionRunner* runner);

  void DeleteFrame(int64_t frame_id);
  void DeleteAllFrames();

 private:
  typedef std::map<std::string, XWalkExtensionRunner*> RunnerMap;
  typedef std::map<int64_t, RunnerMap*> RunnersForFrameMap;
  RunnersForFrameMap runners_for_frame_;

  void DeleteRunnerMap(RunnerMap* runners);

  typedef std::map<const XWalkExtensionRunner*, int64_t> FrameForRunnerMap;
  FrameForRunnerMap frame_for_runner_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionRunnerStore);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_RUNNER_STORE_H_
