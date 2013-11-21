// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_EVENT_ROUTER_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_EVENT_ROUTER_H_

#include <deque>
#include <map>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "base/gtest_prod_util.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/observer_list.h"
#include "base/values.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/application/browser/event_observer.h"

namespace content {
class WebContents;
}

namespace xwalk {
namespace application {

class ApplicationSystem;

// Per application event router. It will created when the application is loaded,
// and destructed when the applicaiton is unloaded.
class ApplicationEventRouter : public content::WebContentsObserver {
 public:
  ApplicationEventRouter(ApplicationSystem* system, const std::string& app_id);
  virtual ~ApplicationEventRouter();

  // Implement content::WebContentsObserver.
  virtual void DidStopLoading(
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void RenderProcessGone(base::TerminationStatus status) OVERRIDE;

  void ObserveMainDocument(content::WebContents* contents);

  void SetMainEvents(const std::set<std::string>& events);
  bool ContainsMainEvent(const std::string& event) const;

  void AttachObserver(const std::string& event_name, EventObserver* observer);
  void DetachObserver(const std::string& event_name, EventObserver* observer);
  void DetachObserver(EventObserver* observer);

  // If the application is not launched or not finish loading the main document
  // the |event| will be regarded as lazy event and queued for later processing.
  void DispatchEvent(scoped_refptr<Event> event);

 private:
  friend class ApplicationEventRouterTest;
  FRIEND_TEST_ALL_PREFIXES(ApplicationEventRouterTest, DetachObservers);

  void DetachObserverFromEvent(const std::string& event_name,
                               EventObserver* observer);
  void DetachAllObservers();

  void ProcessEvent(scoped_refptr<Event> event);
  void ProcessLazyEvents();

  // Key by event name.
  typedef std::map<std::string, linked_ptr<ObserverList<EventObserver> > >
    ObserverListMap;
  // All attached observers.
  ObserverListMap observers_;

  typedef std::vector<scoped_refptr<Event> > EventVector;
  // Lazy events queued before application launched.
  EventVector lazy_events_;

  typedef std::set<std::string> EventSet;
  // Events registered in main document, will be filled when application is
  // loaded.
  EventSet main_events_;

  // True when application's main document or entry page is finished loading.
  bool application_launched_;

  ApplicationSystem* system_;
  std::string app_id_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationEventRouter);
};

}  // namespace application
}  // namespace xwalk
#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_EVENT_ROUTER_H_
