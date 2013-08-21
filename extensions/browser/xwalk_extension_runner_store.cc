// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_runner_store.h"

#include "base/logging.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace xwalk {
namespace extensions {

void XWalkExtensionRunnerStore::AddFrame(int64_t frame_id) {
  CHECK(runners_for_frame_.find(frame_id) == runners_for_frame_.end());
  runners_for_frame_[frame_id] = new RunnerMap;
}

void XWalkExtensionRunnerStore::AddRunner(int64_t frame_id,
                                          XWalkExtensionRunner* runner) {
  CHECK(runners_for_frame_.find(frame_id) != runners_for_frame_.end());
  RunnerMap* runners = runners_for_frame_[frame_id];
  (*runners)[runner->extension_name()] = runner;
  frame_for_runner_[runner] = frame_id;
}

XWalkExtensionRunner* XWalkExtensionRunnerStore::GetRunnerByFrameAndName(
    int64_t frame_id, const std::string& name) {
  RunnerMap* runners = runners_for_frame_[frame_id];
  RunnerMap::iterator it = runners->find(name);
  if (it == runners->end())
    return NULL;
  return it->second;
}

int64_t XWalkExtensionRunnerStore::GetFrameForRunner(
    const XWalkExtensionRunner* runner) {
  return frame_for_runner_[runner];
}

void XWalkExtensionRunnerStore::DeleteFrame(int64_t frame_id) {
  RunnersForFrameMap::iterator it = runners_for_frame_.find(frame_id);
  if (it == runners_for_frame_.end())
    return;
  DeleteRunnerMap(it->second);
  delete it->second;
  runners_for_frame_.erase(it);
}

void XWalkExtensionRunnerStore::DeleteAllFrames() {
  RunnersForFrameMap::iterator it = runners_for_frame_.begin();
  for (; it != runners_for_frame_.end(); ++it) {
    DeleteRunnerMap(it->second);
    delete it->second;
  }
  runners_for_frame_.clear();
}

void XWalkExtensionRunnerStore::DeleteRunnerMap(RunnerMap* runners) {
  RunnerMap::iterator it = runners->begin();
  for (; it != runners->end(); ++it) {
    frame_for_runner_.erase(it->second);
    delete it->second;
  }
  runners->clear();
}

}  // namespace extensions
}  // namespace xwalk
