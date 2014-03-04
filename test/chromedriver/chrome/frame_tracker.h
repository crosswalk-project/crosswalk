// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_FRAME_TRACKER_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_FRAME_TRACKER_H_

#include <map>
#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "chrome/test/chromedriver/chrome/devtools_event_listener.h"

namespace base {
class DictionaryValue;
class Value;
}

class DevToolsClient;
class Status;

// Tracks execution context creation.
class FrameTracker : public DevToolsEventListener {
 public:
  explicit FrameTracker(DevToolsClient* client);
  virtual ~FrameTracker();

  Status GetFrameForContextId(int context_id, std::string* frame_id);
  Status GetContextIdForFrame(const std::string& frame_id, int* context_id);

  // Overridden from DevToolsEventListener:
  virtual Status OnConnected(DevToolsClient* client) OVERRIDE;
  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE;

 private:
  std::map<std::string, int> frame_to_context_map_;

  DISALLOW_COPY_AND_ASSIGN(FrameTracker);
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_FRAME_TRACKER_H_
