// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_EVENT_MANAGER_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_EVENT_MANAGER_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/gtest_prod_util.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"
#include "xwalk/application/browser/event_observer.h"

namespace content {
class WebContents;
}

namespace xwalk {
namespace application {

class ApplicationEventRouter;
class ApplicationSystem;

class Event : public base::RefCounted<Event> {
 public:
  static scoped_refptr<Event> CreateEvent(
      const std::string& event_name, scoped_ptr<base::ListValue> event_args);

  const std::string& name() const { return name_; }
  base::ListValue* args() const { return args_.get(); }

 private:
  friend class base::RefCounted<Event>;
  Event(const std::string& event_name, scoped_ptr<base::ListValue> event_args);
  ~Event();

  // The event to dispatch.
  std::string name_;
  // Arguments to send to the event handler.
  scoped_ptr<base::ListValue> args_;
};

// This's the service class manages all application event routers.
class ApplicationEventManager {
 public:
  explicit ApplicationEventManager(ApplicationSystem* system);
  ~ApplicationEventManager();

  // Create app router when app is loaded.
  void OnAppLoaded(const std::string& app_id);
  // Destroy app router when app is unloaded.
  void OnAppUnloaded(const std::string& app_id);

  void SendEvent(const std::string& app_id,
                 scoped_refptr<Event> event);
  // TODO(xiang): Dispatch event to all loaded applications.
  void BroadcastEvent(scoped_refptr<Event> event) { NOTIMPLEMENTED(); }

  void AttachObserver(const std::string& app_id,
                      const std::string& event_name,
                      EventObserver* observer);
  void DetachObserver(const std::string& app_id,
                      const std::string& event_name,
                      EventObserver* observer);
  void DetachObserver(EventObserver* observer);

  void OnMainDocumentCreated(const std::string& app_id,
                             content::WebContents* contents);

  // TODO(xiang): Get events registered in main document.
  const std::set<std::string> GetAppMainEvents(const std::string& app_id);
  // TODO(xiang): save these events to backing store at app installation time.
  void SetAppMainEvents(const std::string& app_id,
                        const std::set<std::string>& events);

 private:
  ApplicationEventRouter* GetAppRouter(const std::string& app_id);

  typedef std::map<std::string, linked_ptr<ApplicationEventRouter> >
      AppRouterMap;
  AppRouterMap app_routers_;

  ApplicationSystem* system_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationEventManager);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_EVENT_MANAGER_H_
