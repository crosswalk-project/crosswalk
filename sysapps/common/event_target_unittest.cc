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

  // The JavaScript shim should take care of tracking
  // events of a given object and sending only one message
  // activating the event. Sending multiple messages will
  // print a warning. Same goes for deactivating.
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
  EventTargetTest* target(new EventTargetTest());
  scoped_ptr<BindingObject> target_ptr(target);

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
