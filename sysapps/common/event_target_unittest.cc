// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/event_target.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

using xwalk::extensions::XWalkExtensionFunctionInfo;
using xwalk::sysapps::BindingObject;
using xwalk::sysapps::EventTarget;

namespace {

const char kTestString[] = "crosswalk1234567890";

void DummyCallback(scoped_ptr<base::ListValue> result) {}

scoped_ptr<XWalkExtensionFunctionInfo> CreateFunctionInfo(
    const std::string& name, const std::string& str_argument) {
  scoped_ptr<base::ListValue> arguments(new base::ListValue);
  arguments->AppendString(str_argument);

  return make_scoped_ptr(new XWalkExtensionFunctionInfo(
      name,
      arguments.Pass(),
      base::Bind(&DummyCallback)));
}

void DispatchResult(int* message_count, scoped_ptr<base::ListValue> result) {
  std::string test_string;
  result->GetString(0, &test_string);
  EXPECT_EQ(test_string, kTestString);

  (*message_count)++;
}

class EventTargetTest : public EventTarget {
 public:
  EventTargetTest()
      : event1_count_(0),
        event2_count_(0) {}

  void InjectEvent(const std::string& type) {
    scoped_ptr<base::ListValue> data(new base::ListValue());
    data->AppendString(kTestString);

    DispatchEvent(type, data.Pass());
  }

  bool is_event1_active() const {
    return event1_count_ == 1;
  }

  bool is_event2_active() const {
    return event2_count_ == 1;
  }

 private:
  virtual void StartEvent(const std::string& type) OVERRIDE {
    if (type == "event1")
      event1_count_++;
    if (type == "event2")
      event2_count_++;

    // An event should never be started or stopped twice, so only two states are
    // possible here: 0 (event is not active) and 1 (event is active). If we see
    // any other value (say -1 or 2), it means that StartEvent or StopEvent was
    // called twice for a given event and it should never happen.
    EXPECT_TRUE(event1_count_ == 0 || event1_count_ == 1);
    EXPECT_TRUE(event2_count_ == 0 || event2_count_ == 1);
  }

  virtual void StopEvent(const std::string& type) OVERRIDE {
    if (type == "event1")
      event1_count_--;
    if (type == "event2")
      event2_count_--;

    EXPECT_TRUE(event1_count_ == 0 || event1_count_ == 1);
    EXPECT_TRUE(event2_count_ == 0 || event2_count_ == 1);
  }

  int event1_count_;
  int event2_count_;
};

}  // namespace

TEST(XWalkSysAppsEventTargetTest, StartStopEvent) {
  scoped_ptr<EventTargetTest> target(new EventTargetTest());

  // The JS shim should do the listeners management and call "addEventListener"
  // only when the first listener is added and "removeEventListener" only when
  // the last is removed. Multiple calls of "addEventListener" for the same
  // event will print a warning.
  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("addEventListener", "event1")));
  EXPECT_TRUE(target->is_event1_active());

  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("addEventListener", "event1")));
  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("addEventListener", "event1")));
  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("addEventListener", "event1")));
  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("addEventListener", "event1")));
  EXPECT_TRUE(target->is_event1_active());

  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("removeEventListener", "event1")));
  EXPECT_FALSE(target->is_event1_active());

  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("removeEventListener", "event1")));
  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("removeEventListener", "event1")));
  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("removeEventListener", "event1")));
  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("removeEventListener", "event1")));
  EXPECT_FALSE(target->is_event1_active());

  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("addEventListener", "event2")));
  EXPECT_TRUE(target->is_event2_active());

  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("removeEventListener", "event2")));
  EXPECT_FALSE(target->is_event2_active());

  // Removing a event that was never added should
  // only issue a warning message.
  EXPECT_TRUE(target->HandleFunction(
          CreateFunctionInfo("removeEventListener", "event3")));
}

TEST(XWalkSysAppsEventTargetTest, DispatchEvent) {
  scoped_ptr<EventTargetTest> target(new EventTargetTest());

  target->InjectEvent("event1");
  target->InjectEvent("event1");
  target->InjectEvent("event1");

  int message_count = 0;

  scoped_ptr<base::ListValue> argumentsList(new base::ListValue);
  argumentsList->AppendString("event1");

  scoped_ptr<XWalkExtensionFunctionInfo> eventInfo(
      new XWalkExtensionFunctionInfo(
          "addEventListener",
          argumentsList.Pass(),
          base::Bind(&DispatchResult, &message_count)));

  EXPECT_TRUE(target->HandleFunction(eventInfo.Pass()));

  for (unsigned i = 0; i < 1000; ++i) {
    target->InjectEvent("event1");
    EXPECT_EQ(message_count, i + 1);
  }
}
