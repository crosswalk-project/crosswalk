// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_HEAP_SNAPSHOT_TAKER_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_HEAP_SNAPSHOT_TAKER_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/chromedriver/chrome/devtools_event_listener.h"

namespace base {
class DictionaryValue;
class Value;
}

class DevToolsClient;
class Status;

// Take the heap snapshot.
class HeapSnapshotTaker: public DevToolsEventListener {
 public:
  explicit HeapSnapshotTaker(DevToolsClient* client);
  virtual ~HeapSnapshotTaker();

  Status TakeSnapshot(scoped_ptr<base::Value>* snapshot);

  // Overridden from DevToolsEventListener:
  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE;

 private:
  Status TakeSnapshotInternal();

  DevToolsClient* client_;
  int snapshot_uid_;
  std::string snapshot_;

  DISALLOW_COPY_AND_ASSIGN(HeapSnapshotTaker);
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_HEAP_SNAPSHOT_TAKER_H_
