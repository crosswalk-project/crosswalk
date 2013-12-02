// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/event_observer.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/application/test/application_apitest.h"
#include "xwalk/application/test/application_testapi.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"

using xwalk::application::ApplicationEventManager;
using xwalk::application::Event;
using xwalk::application::kOnJavaScriptEventAck;

namespace {

const char kMockEvent[] = "onMockEvent";

}  // namespace

class ApplicationEventApiTest : public ApplicationApiTest {
 public:
  ApplicationEventApiTest()
    : event_manager_(NULL) {
  }

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    ApplicationBrowserTest::SetUpCommandLine(command_line);
    GURL url = net::FilePathToFileURL(test_data_dir_.Append(
          FILE_PATH_LITERAL("eventapi")));
    command_line->AppendArg(url.spec());
  }

  void PrepareFinishObserver() {
    xwalk::Runtime* main_runtime = xwalk::RuntimeRegistry::Get()->runtimes()[0];
    xwalk::RuntimeContext* runtime_context = main_runtime->runtime_context();
    xwalk::application::ApplicationSystem* system =
      runtime_context->GetApplicationSystem();
    DCHECK(system->application_service()->GetRunningApplication());

    app_id_ = system->application_service()->GetRunningApplication()->ID();
    event_manager_ = system->event_manager();
    event_finish_observer_.reset(
        new MockFinishObserver(event_manager_, app_id_));
  }

  // FIXME(xiang): we need to destruct the observer early, because when
  // ApplicationEventManager::DetachObserver called from test case destruction
  // the check for running on UI thread will fail even it runs on UI thread.
  void CloseFinishObserver() {
    event_finish_observer_.release();
  }

  void SendEvent() {
    DCHECK(event_manager_);
    scoped_ptr<base::ListValue> args(new base::ListValue());
    args->AppendInteger(1234);
    args->AppendBoolean(false);
    scoped_refptr<Event> event = Event::CreateEvent(kMockEvent, args.Pass());
    event_manager_->SendEvent(app_id_, event);
  }

  bool EventFinishObserverNotified() {
    return event_finish_observer_->has_been_notified_;
  }

 private:
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

  std::string app_id_;
  ApplicationEventManager* event_manager_;
  scoped_ptr<MockFinishObserver> event_finish_observer_;
};

IN_PROC_BROWSER_TEST_F(ApplicationEventApiTest, EventApiTest) {
  test_runner_->WaitForTestComplete();
  ASSERT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);

  PrepareFinishObserver();
  test_runner_->ResetResult();

  // Send event to test JS listeners and event finish observer.
  SendEvent();
  test_runner_->WaitForTestComplete();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
  content::RunAllPendingInMessageLoop();
  EXPECT_TRUE(EventFinishObserverNotified());
  CloseFinishObserver();
}
