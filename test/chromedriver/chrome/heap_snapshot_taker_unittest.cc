// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <list>
#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/chrome/heap_snapshot_taker.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/chrome/stub_devtools_client.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const char* chunks[] = {"{\"a\": 1,", "\"b\": 2}"};

scoped_ptr<base::Value> GetSnapshotAsValue() {
  scoped_ptr<base::DictionaryValue> dict(new base::DictionaryValue());
  dict->SetInteger("a", 1);
  dict->SetInteger("b", 2);
  return dict.PassAs<base::Value>();
}

class DummyDevToolsClient : public StubDevToolsClient {
 public:
  DummyDevToolsClient(const std::string& method, bool error_after_events)
      : method_(method),
        error_after_events_(error_after_events),
        uid_(1),
        cleared_(false),
        disabled_(false) {}
  virtual ~DummyDevToolsClient() {}

  bool IsCleared() { return cleared_; }

  bool IsDisabled() { return disabled_; }

  virtual Status SendAddProfileHeaderEvent() {
    base::DictionaryValue event_params;
    event_params.SetInteger("header.uid", uid_);
    return listeners_.front()->OnEvent(
        this, "HeapProfiler.addProfileHeader", event_params);
  }

  virtual Status SendAddHeapSnapshotChunkEvent() {
    base::DictionaryValue event_params;
    event_params.SetInteger("uid", uid_);
    for (size_t i = 0; i < arraysize(chunks); ++i) {
      event_params.SetString("chunk", chunks[i]);
      Status status = listeners_.front()->OnEvent(
          this, "HeapProfiler.addHeapSnapshotChunk", event_params);
      if (status.IsError())
        return status;
    }
    return Status(kOk);
  }

  // Overridden from DevToolsClient:
  virtual Status SendCommand(const std::string& method,
                             const base::DictionaryValue& params) OVERRIDE {
    if (!cleared_)
      cleared_ = method == "HeapProfiler.clearProfiles";
    if (!disabled_)
      disabled_ = method == "Debugger.disable";
    if (method == method_ && !error_after_events_)
      return Status(kUnknownError);

    if (method == "HeapProfiler.takeHeapSnapshot") {
      Status status = SendAddProfileHeaderEvent();
      if (status.IsError())
        return status;
    } else if (method == "HeapProfiler.getHeapSnapshot") {
      Status status = SendAddHeapSnapshotChunkEvent();
      if (status.IsError())
        return status;
    }

    if (method == method_ && error_after_events_)
      return Status(kUnknownError);
    return StubDevToolsClient::SendCommand(method, params);
  }

 protected:
  std::string method_;  // Throw error on command with this method.
  bool error_after_events_;
  int uid_;
  bool cleared_;  // True if HeapProfiler.clearProfiles was issued.
  bool disabled_;  // True if Debugger.disable was issued.
};

}  // namespace

TEST(HeapSnapshotTaker, SuccessfulCase) {
  DummyDevToolsClient client("", false);
  HeapSnapshotTaker taker(&client);
  scoped_ptr<base::Value> snapshot;
  Status status = taker.TakeSnapshot(&snapshot);
  ASSERT_EQ(kOk, status.code());
  ASSERT_TRUE(GetSnapshotAsValue()->Equals(snapshot.get()));
  ASSERT_TRUE(client.IsCleared());
  ASSERT_TRUE(client.IsDisabled());
}

TEST(HeapSnapshotTaker, FailIfErrorOnDebuggerEnable) {
  DummyDevToolsClient client("Debugger.enable", false);
  HeapSnapshotTaker taker(&client);
  scoped_ptr<base::Value> snapshot;
  Status status = taker.TakeSnapshot(&snapshot);
  ASSERT_TRUE(status.IsError());
  ASSERT_FALSE(snapshot.get());
  ASSERT_FALSE(client.IsCleared());
  ASSERT_TRUE(client.IsDisabled());
}

TEST(HeapSnapshotTaker, FailIfErrorOnCollectGarbage) {
  DummyDevToolsClient client("HeapProfiler.collectGarbage", false);
  HeapSnapshotTaker taker(&client);
  scoped_ptr<base::Value> snapshot;
  Status status = taker.TakeSnapshot(&snapshot);
  ASSERT_TRUE(status.IsError());
  ASSERT_FALSE(snapshot.get());
  ASSERT_FALSE(client.IsCleared());
  ASSERT_TRUE(client.IsDisabled());
}

TEST(HeapSnapshotTaker, ErrorBeforeReceivingUid) {
  DummyDevToolsClient client("HeapProfiler.takeHeapSnapshot", false);
  HeapSnapshotTaker taker(&client);
  scoped_ptr<base::Value> snapshot;
  Status status = taker.TakeSnapshot(&snapshot);
  ASSERT_TRUE(status.IsError());
  ASSERT_FALSE(snapshot.get());
  ASSERT_FALSE(client.IsCleared());
  ASSERT_TRUE(client.IsDisabled());
}

TEST(HeapSnapshotTaker, ErrorAfterReceivingUid) {
  DummyDevToolsClient client("HeapProfiler.takeHeapSnapshot", true);
  HeapSnapshotTaker taker(&client);
  scoped_ptr<base::Value> snapshot;
  Status status = taker.TakeSnapshot(&snapshot);
  ASSERT_TRUE(status.IsError());
  ASSERT_FALSE(snapshot.get());
  ASSERT_TRUE(client.IsCleared());
  ASSERT_TRUE(client.IsDisabled());
}

namespace {

class TwoUidEventClient : public DummyDevToolsClient {
 public:
  TwoUidEventClient() : DummyDevToolsClient("", false) {}
  virtual ~TwoUidEventClient() {}

  // Overridden from DummyDevToolsClient:
  virtual Status SendAddProfileHeaderEvent() OVERRIDE {
    Status status = DummyDevToolsClient::SendAddProfileHeaderEvent();
    if (status.IsError())
      return status;
    uid_ = 2;
    status = DummyDevToolsClient::SendAddProfileHeaderEvent();
    uid_ = 1;
    return status;
  }
};

}  // namespace

TEST(HeapSnapshotTaker, MuiltipleUidEvents) {
  TwoUidEventClient client;
  HeapSnapshotTaker taker(&client);
  scoped_ptr<base::Value> snapshot;
  Status status = taker.TakeSnapshot(&snapshot);
  ASSERT_EQ(kOk, status.code());
  ASSERT_TRUE(GetSnapshotAsValue()->Equals(snapshot.get()));
  ASSERT_TRUE(client.IsCleared());
  ASSERT_TRUE(client.IsDisabled());
}

namespace {

class ChunkWithDifferentUidClient : public DummyDevToolsClient {
 public:
  ChunkWithDifferentUidClient() : DummyDevToolsClient("", false) {}
  virtual ~ChunkWithDifferentUidClient() {}

  // Overridden from DummyDevToolsClient:
  virtual Status SendAddHeapSnapshotChunkEvent() OVERRIDE {
    Status status = DummyDevToolsClient::SendAddHeapSnapshotChunkEvent();
    if (status.IsError())
      return status;
    uid_ = 2;
    status = DummyDevToolsClient::SendAddHeapSnapshotChunkEvent();
    uid_ = 1;
    return status;
  }
};

}  // namespace

TEST(HeapSnapshotTaker, IgnoreChunkWithDifferentUid) {
  ChunkWithDifferentUidClient client;
  HeapSnapshotTaker taker(&client);
  scoped_ptr<base::Value> snapshot;
  Status status = taker.TakeSnapshot(&snapshot);
  ASSERT_EQ(kOk, status.code());
  ASSERT_TRUE(GetSnapshotAsValue()->Equals(snapshot.get()));
  ASSERT_TRUE(client.IsCleared());
  ASSERT_TRUE(client.IsDisabled());
}

TEST(HeapSnapshotTaker, ErrorAfterFinishEvent) {
  DummyDevToolsClient client("HeapProfiler.getHeapSnapshot", true);
  HeapSnapshotTaker taker(&client);
  scoped_ptr<base::Value> snapshot;
  Status status = taker.TakeSnapshot(&snapshot);
  ASSERT_TRUE(status.IsError());
  ASSERT_FALSE(snapshot.get());
  ASSERT_TRUE(client.IsCleared());
  ASSERT_TRUE(client.IsDisabled());
}
