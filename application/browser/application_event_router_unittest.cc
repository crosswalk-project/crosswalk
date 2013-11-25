// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_event_router.h"
#include "xwalk/application/browser/event_observer.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {
namespace application {

namespace {

const char kMockEvent0[] = "MOCK_EVENT_0";
const char kMockEvent1[] = "MOCK_EVENT_1";
const char kMockAppId0[] = "mock_app_0";

std::vector<std::string> g_call_sequence;

class MockEventObserver : public EventObserver {
 public:
  explicit MockEventObserver(ApplicationEventManager* manager)
    : EventObserver(manager) {
  }

  virtual void Observe(const std::string& app_id,
                       scoped_refptr<Event> event) OVERRIDE {
    if (event->name() == kMockEvent0 || event->name() == kMockEvent1) {
      HandleEvent(app_id, event);
      return;
    }
    NOTREACHED();
  }

  void HandleEvent(const std::string& app_id, scoped_refptr<Event> event) {
    std::string call_info = __FUNCTION__;
    call_info += "," + app_id + "," + event->name();
    g_call_sequence.push_back(call_info);
  }
};

}  // namespace

class ApplicationEventRouterTest : public testing::Test {
 public:
  virtual void SetUp() OVERRIDE {
    runtime_context_.reset(new xwalk::RuntimeContext);
    system_.reset(new ApplicationSystem(runtime_context_.get()));
    router_.reset(new ApplicationEventRouter(system_.get(), kMockAppId0));
    event_manager_ = system_->event_manager();
  }

  void SendEventToApp(const std::string& event_name) {
    scoped_refptr<Event> event = Event::CreateEvent(
        event_name, scoped_ptr<base::ListValue>(new base::ListValue()));
    router_->ProcessEvent(event);
  }

  int GetObserverCount(const std::string& event_name) {
    ApplicationEventRouter::ObserverListMap::iterator it =
        router_->observers_.find(event_name);
    if (it == router_->observers_.end() || !it->second->might_have_observers())
      return 0;
    ObserverList<EventObserver>::Iterator ob_it(*it->second);
    int count = 0;
    while (ob_it.GetNext() != NULL)
      ++count;
    return count;
  }

  int GetRegisteredEventCount() {
    return router_->observers_.size();
  }

 protected:
  ApplicationEventManager* event_manager_;
  scoped_ptr<ApplicationEventRouter> router_;

 private:
  scoped_ptr<xwalk::RuntimeContext> runtime_context_;
  scoped_ptr<ApplicationSystem> system_;
  content::TestBrowserThreadBundle thread_bundle_;
};

TEST_F(ApplicationEventRouterTest, AttachObserver) {
  MockEventObserver observer(event_manager_);

  router_->AttachObserver(kMockEvent0, &observer);
  ASSERT_EQ(GetObserverCount(kMockEvent0), 1);

  router_->AttachObserver(kMockEvent1, &observer);
  ASSERT_EQ(GetRegisteredEventCount(), 2);
  ASSERT_EQ(GetObserverCount(kMockEvent1), 1);
}

TEST_F(ApplicationEventRouterTest, DetachObserverFromEvent) {
  MockEventObserver observer1(event_manager_);
  MockEventObserver observer2(event_manager_);

  router_->AttachObserver(kMockEvent0, &observer1);
  ASSERT_EQ(GetObserverCount(kMockEvent0), 1);
  router_->AttachObserver(kMockEvent0, &observer2);
  ASSERT_EQ(GetObserverCount(kMockEvent0), 2);

  router_->DetachObserver(kMockEvent0, &observer1);
  ASSERT_EQ(GetObserverCount(kMockEvent0), 1);

  // Removing non-exist observer from router must not crash.
  router_->DetachObserver(kMockEvent0, &observer1);
  ASSERT_EQ(GetObserverCount(kMockEvent0), 1);

  router_->DetachObserver(kMockEvent0, &observer2);
  ASSERT_EQ(GetObserverCount(kMockEvent0), 0);
  ASSERT_EQ(GetRegisteredEventCount(), 0);
}

TEST_F(ApplicationEventRouterTest, DetachObservers) {
  MockEventObserver observer1(event_manager_);
  MockEventObserver observer2(event_manager_);
  MockEventObserver observer3(event_manager_);
  router_->AttachObserver(kMockEvent0, &observer1);
  router_->AttachObserver(kMockEvent1, &observer1);
  router_->AttachObserver(kMockEvent1, &observer2);
  router_->AttachObserver(kMockEvent0, &observer3);
  router_->AttachObserver(kMockEvent1, &observer3);
  ASSERT_EQ(GetObserverCount(kMockEvent0), 2);
  ASSERT_EQ(GetObserverCount(kMockEvent1), 3);

  router_->DetachObserver(&observer1);
  ASSERT_EQ(GetObserverCount(kMockEvent0), 1);
  ASSERT_EQ(GetObserverCount(kMockEvent1), 2);

  router_->DetachAllObservers();
  ASSERT_EQ(GetObserverCount(kMockEvent0), 0);
  ASSERT_EQ(GetObserverCount(kMockEvent1), 0);
  ASSERT_EQ(GetRegisteredEventCount(), 0);
}

// Dispatch event which has multiple observers, all observers should be
// notified.
TEST_F(ApplicationEventRouterTest, EventDispatch) {
  MockEventObserver observer1(event_manager_);
  MockEventObserver observer2(event_manager_);
  MockEventObserver observer3(event_manager_);
  g_call_sequence.clear();
  router_->AttachObserver(kMockEvent0, &observer1);
  router_->AttachObserver(kMockEvent0, &observer2);
  router_->AttachObserver(kMockEvent0, &observer3);

  SendEventToApp(kMockEvent0);
  ASSERT_EQ(g_call_sequence.size(), 3);
}

}  // namespace application
}  // namespace xwalk
