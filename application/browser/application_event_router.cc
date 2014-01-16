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
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/event_names.h"

namespace xwalk {
namespace application {

ApplicationEventRouter::ApplicationEventRouter(const std::string& app_id)
    : app_id_(app_id),
      main_document_loaded_(false) {
}

ApplicationEventRouter::~ApplicationEventRouter() {
  DetachAllObservers();
}

void ApplicationEventRouter::DidStopLoading(
    content::RenderViewHost* render_view_host) {
  main_document_loaded_ = true;
  ProcessLazyEvents();
}

void ApplicationEventRouter::RenderProcessGone(
    base::TerminationStatus status) {
  main_document_loaded_ = false;
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
  const std::string& event_name = event->name();

  if (!main_document_loaded_) {
    lazy_events_.push_back(event);
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
  if (ContainsKey(observers_, event_name)) {
    FOR_EACH_OBSERVER(
        EventObserver, *observers_[event_name], Observe(app_id_, event));
  }
}

}  // namespace application
}  // namespace xwalk
