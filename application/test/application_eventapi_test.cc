// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/event_observer.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/application/test/application_apitest.h"
#include "xwalk/application/test/application_testapi.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

using xwalk::application::Application;
using xwalk::application::ApplicationEventManager;
using xwalk::application::Event;
using xwalk::application::kOnJavaScriptEventAck;

namespace {

const char kMockEvent[] = "onMockEvent";

scoped_refptr<Event> CreateMockEvent() {
  scoped_ptr<base::ListValue> args(new base::ListValue());
  args->AppendInteger(1234);
  args->AppendBoolean(false);
  return Event::CreateEvent(kMockEvent, args.Pass());
}

}  // namespace

class ApplicationEventApiTest : public ApplicationApiTest {
 protected:
  class MockFinishObserver : public xwalk::application::EventObserver {
   public:
    MockFinishObserver(
        ApplicationEventManager* event_manager,
        const std::string& app_id)
      : EventObserver(event_manager),
        has_been_notified_(false) {
      event_manager_->AttachObserver(app_id, kOnJavaScriptEventAck, this);
    }

    virtual void Observe(const std::string& app_id,
                         scoped_refptr<Event> event) OVERRIDE {
      ASSERT_EQ(event->name(), std::string(kOnJavaScriptEventAck));
      std::string ack_event_name;
      ASSERT_TRUE(event->args()->GetString(0, &ack_event_name));
      EXPECT_EQ(ack_event_name, std::string(kMockEvent));
      has_been_notified_ = true;
    }

    bool has_been_notified_;
  };
};

IN_PROC_BROWSER_TEST_F(ApplicationEventApiTest, EventApiTest) {
  xwalk::application::ApplicationSystem* system =
      xwalk::XWalkRunner::GetInstance()->app_system();
  Application* app = system->application_service()->Launch(
      test_data_dir_.Append(FILE_PATH_LITERAL("eventapi")));
  ASSERT_TRUE(app);

  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
  test_runner_->PostResultToNotificationCallback();
  scoped_ptr<MockFinishObserver> event_finish_observer(
      new MockFinishObserver(system->event_manager(), app->id()));

  system->event_manager()->SendEvent(app->id(), CreateMockEvent());
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
  EXPECT_TRUE(event_finish_observer->has_been_notified_);
}
