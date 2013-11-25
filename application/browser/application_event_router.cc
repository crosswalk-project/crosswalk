// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_event_router.h"

#include <algorithm>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"

namespace xwalk {
namespace application {

ApplicationEventRouter::ApplicationEventRouter(
    ApplicationSystem* system, const std::string& app_id)
    : system_(system),
      app_id_(app_id),
      application_launched_(false) {
}

ApplicationEventRouter::~ApplicationEventRouter() {
  DetachAllObservers();
}

void ApplicationEventRouter::DidStopLoading(
    content::RenderViewHost* render_view_host) {
  application_launched_ = true;
  ProcessLazyEvents();
}

void ApplicationEventRouter::RenderProcessGone(
    base::TerminationStatus status) {
  application_launched_ = false;
  DetachAllObservers();
}

void ApplicationEventRouter::ObserveMainDocument(
    content::WebContents* contents) {
  content::WebContentsObserver::Observe(contents);
}

void ApplicationEventRouter::SetMainEvents(
    const std::set<std::string>& events) {
  main_events_ = events;
}

void ApplicationEventRouter::AttachObserver(const std::string& event_name,
                                            EventObserver* observer) {
  if (!ContainsKey(observers_, event_name)) {
    observers_.insert(std::make_pair(
          event_name, new ObserverList<EventObserver>()));
  }
  observers_[event_name]->AddObserver(observer);
}

void ApplicationEventRouter::DetachObserver(const std::string& event_name,
                                            EventObserver* observer) {
  if (!ContainsKey(observers_, event_name)) {
    DLOG(WARNING) << "Can't find attached observer for application(" << app_id_
                  << ") with event(" << event_name << ").";
    return;
  }

  observers_[event_name]->RemoveObserver(observer);
  if (!observers_[event_name]->might_have_observers()) {
    observers_.erase(event_name);
  }
}

void ApplicationEventRouter::DetachObserver(EventObserver* observer) {
  ObserverListMap::iterator it = observers_.begin();
  while (it != observers_.end())
    DetachObserver((it++)->first, observer);
}

void ApplicationEventRouter::DispatchEvent(scoped_refptr<Event> event) {
  ApplicationService* service = system_->application_service();
  const std::string& event_name = event->name();

  if (main_events_.find(event_name) != main_events_.end() ||
      event_name == "app.onLaunched") {
    if (!application_launched_) {
      if (lazy_events_.empty() && !service->Launch(app_id_))
        LOG(WARNING) << "Fail to launch application:" << app_id_;
      else
        lazy_events_.push_back(event);
      return;
    }
  }

  if (!application_launched_) {
    LOG(WARNING) << "Ignore event: " << event_name
                 << " send to terminated application:" << app_id_;
    return;
  }

  if (!ContainsKey(observers_, event_name) ||
      !observers_[event_name]->might_have_observers()) {
    LOG(INFO) << "No registered handler to handle event:" << event_name;
    return;
  }

  ProcessEvent(event);
}

void ApplicationEventRouter::DetachAllObservers() {
  ObserverListMap::iterator it = observers_.begin();
  for (; it != observers_.end(); ++it) {
    it->second->Clear();
  }
  observers_.clear();
}

void ApplicationEventRouter::ProcessLazyEvents() {
  if (lazy_events_.empty())
    return;

  EventVector::iterator it = lazy_events_.begin();
  for (; it != lazy_events_.end(); ++it)
    ProcessEvent(*it);
  lazy_events_.clear();
}

void ApplicationEventRouter::ProcessEvent(scoped_refptr<Event> event) {
  const std::string& event_name = event->name();
  DCHECK(ContainsKey(observers_, event_name));
  FOR_EACH_OBSERVER(
      EventObserver, *observers_[event_name], Observe(app_id_, event));
}

}  // namespace application
}  // namespace xwalk
