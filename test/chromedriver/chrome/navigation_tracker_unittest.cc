// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/compiler_specific.h"
#include "base/json/json_reader.h"
#include "base/values.h"
#include "chrome/test/chromedriver/chrome/navigation_tracker.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome/stub_devtools_client.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

void AssertPendingState(NavigationTracker* tracker,
                        const std::string& frame_id,
                        bool expected_is_pending) {
  bool is_pending = !expected_is_pending;
  ASSERT_EQ(kOk, tracker->IsPendingNavigation(frame_id, &is_pending).code());
  ASSERT_EQ(expected_is_pending, is_pending);
}

}  // namespace

TEST(NavigationTracker, FrameLoadStartStop) {
  StubDevToolsClient client;
  NavigationTracker tracker(&client);

  base::DictionaryValue params;
  ASSERT_EQ(
      kOk, tracker.OnEvent(&client, "Page.frameStartedLoading", params).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", true));
  ASSERT_EQ(
      kOk, tracker.OnEvent(&client, "Page.frameStoppedLoading", params).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", false));
}

TEST(NavigationTracker, NavigationScheduledThenLoaded) {
  StubDevToolsClient client;
  NavigationTracker tracker(&client, NavigationTracker::kNotLoading);
  base::DictionaryValue params;
  params.SetString("frameId", "f");
  base::DictionaryValue params_scheduled;
  params_scheduled.SetInteger("delay", 0);
  params_scheduled.SetString("frameId", "f");

  ASSERT_EQ(
      kOk,
      tracker.OnEvent(
          &client, "Page.frameScheduledNavigation", params_scheduled).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", true));
  ASSERT_EQ(
      kOk, tracker.OnEvent(&client, "Page.frameStartedLoading", params).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", true));
  ASSERT_EQ(
      kOk,
      tracker.OnEvent(&client, "Page.frameClearedScheduledNavigation", params)
          .code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", true));
  ASSERT_EQ(
      kOk, tracker.OnEvent(&client, "Page.frameStoppedLoading", params).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", false));
}

TEST(NavigationTracker, NavigationScheduledForOtherFrame) {
  StubDevToolsClient client;
  NavigationTracker tracker(&client, NavigationTracker::kNotLoading);
  base::DictionaryValue params_scheduled;
  params_scheduled.SetInteger("delay", 0);
  params_scheduled.SetString("frameId", "other");

  ASSERT_EQ(
      kOk,
      tracker.OnEvent(
          &client, "Page.frameScheduledNavigation", params_scheduled).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", false));
}

TEST(NavigationTracker, NavigationScheduledThenCancelled) {
  StubDevToolsClient client;
  NavigationTracker tracker(&client, NavigationTracker::kNotLoading);
  base::DictionaryValue params;
  params.SetString("frameId", "f");
  base::DictionaryValue params_scheduled;
  params_scheduled.SetInteger("delay", 0);
  params_scheduled.SetString("frameId", "f");

  ASSERT_EQ(
      kOk,
      tracker.OnEvent(
          &client, "Page.frameScheduledNavigation", params_scheduled).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", true));
  ASSERT_EQ(
      kOk,
      tracker.OnEvent(&client, "Page.frameClearedScheduledNavigation", params)
          .code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", false));
}

TEST(NavigationTracker, NavigationScheduledTooFarAway) {
  StubDevToolsClient client;
  NavigationTracker tracker(&client, NavigationTracker::kNotLoading);

  base::DictionaryValue params_scheduled;
  params_scheduled.SetInteger("delay", 10);
  params_scheduled.SetString("frameId", "f");
  ASSERT_EQ(
      kOk,
      tracker.OnEvent(
          &client, "Page.frameScheduledNavigation", params_scheduled).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", false));
}

TEST(NavigationTracker, DiscardScheduledNavigationsOnMainFrameCommit) {
  StubDevToolsClient client;
  NavigationTracker tracker(&client, NavigationTracker::kNotLoading);

  base::DictionaryValue params_scheduled;
  params_scheduled.SetString("frameId", "subframe");
  params_scheduled.SetInteger("delay", 0);
  ASSERT_EQ(
      kOk,
      tracker.OnEvent(
          &client, "Page.frameScheduledNavigation", params_scheduled).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "subframe", true));

  base::DictionaryValue params_navigated;
  params_navigated.SetString("frame.parentId", "something");
  ASSERT_EQ(
      kOk,
      tracker.OnEvent(&client, "Page.frameNavigated", params_navigated).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "subframe", true));
  params_navigated.Clear();
  ASSERT_EQ(
      kOk,
      tracker.OnEvent(&client, "Page.frameNavigated", params_navigated).code());
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "subframe", false));
}

namespace {

class FailToEvalScriptDevToolsClient : public StubDevToolsClient {
 public:
  virtual ~FailToEvalScriptDevToolsClient() {}

  virtual Status SendCommandAndGetResult(
      const std::string& method,
      const base::DictionaryValue& params,
      scoped_ptr<base::DictionaryValue>* result) OVERRIDE {
    EXPECT_STREQ("Runtime.evaluate", method.c_str());
    return Status(kUnknownError, "failed to eval script");
  }
};

}  // namespace

TEST(NavigationTracker, UnknownStateFailsToDetermineState) {
  FailToEvalScriptDevToolsClient client;
  NavigationTracker tracker(&client);
  bool is_pending;
  ASSERT_EQ(kUnknownError,
            tracker.IsPendingNavigation("f", &is_pending).code());
}

namespace {

class DeterminingLoadStateDevToolsClient : public StubDevToolsClient {
 public:
  DeterminingLoadStateDevToolsClient(
      bool is_loading,
      const std::string& send_event_first,
      base::DictionaryValue* send_event_first_params)
      : is_loading_(is_loading),
        send_event_first_(send_event_first),
        send_event_first_params_(send_event_first_params) {}

  virtual ~DeterminingLoadStateDevToolsClient() {}

  virtual Status SendCommandAndGetResult(
      const std::string& method,
      const base::DictionaryValue& params,
      scoped_ptr<base::DictionaryValue>* result) OVERRIDE {
    if (send_event_first_.length()) {
      Status status = listeners_.front()
          ->OnEvent(this, send_event_first_, *send_event_first_params_);
      if (status.IsError())
        return status;
    }

    base::DictionaryValue result_dict;
    result_dict.SetBoolean("result.value", is_loading_);
    result->reset(result_dict.DeepCopy());
    return Status(kOk);
  }

 private:
  bool is_loading_;
  std::string send_event_first_;
  base::DictionaryValue* send_event_first_params_;
};

}  // namespace

TEST(NavigationTracker, UnknownStateForcesStart) {
  base::DictionaryValue params;
  DeterminingLoadStateDevToolsClient client(true, std::string(), &params);
  NavigationTracker tracker(&client);
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", true));
}

TEST(NavigationTracker, UnknownStateForcesStartReceivesStop) {
  base::DictionaryValue params;
  DeterminingLoadStateDevToolsClient client(
      true, "Page.frameStoppedLoading", &params);
  NavigationTracker tracker(&client);
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", false));
}

TEST(NavigationTracker, OnSuccessfulNavigate) {
  base::DictionaryValue params;
  DeterminingLoadStateDevToolsClient client(
      true, "Page.frameStoppedLoading", &params);
  NavigationTracker tracker(&client, NavigationTracker::kNotLoading);
  tracker.OnCommandSuccess(&client, "Page.navigate");
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", false));
}

TEST(NavigationTracker, OnSuccessfulNavigateStillWaiting) {
  base::DictionaryValue params;
  DeterminingLoadStateDevToolsClient client(true, std::string(), &params);
  NavigationTracker tracker(&client, NavigationTracker::kNotLoading);
  tracker.OnCommandSuccess(&client, "Page.navigate");
  ASSERT_NO_FATAL_FAILURE(AssertPendingState(&tracker, "f", true));
}
