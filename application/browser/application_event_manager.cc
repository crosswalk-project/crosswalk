// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_event_manager.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/application/browser/application_event_router.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_system.h"

using content::BrowserThread;

namespace xwalk {
namespace application {

scoped_refptr<Event> Event::CreateEvent(
    const std::string& event_name, scoped_ptr<base::ListValue> event_args) {
  return scoped_refptr<Event>(new Event(event_name, event_args.Pass()));
}

Event::Event(const std::string& event_name,
             scoped_ptr<base::ListValue> event_args)
  : name_(event_name),
    args_(event_args.Pass()) {
  DCHECK(args_);
}

Event::~Event() {
}

ApplicationEventManager::ApplicationEventManager(ApplicationSystem* system)
  : system_(system) {
}

ApplicationEventManager::~ApplicationEventManager() {
}

void ApplicationEventManager::OnAppLoaded(const std::string& app_id) {
  linked_ptr<ApplicationEventRouter> router(
      new ApplicationEventRouter(system_, app_id));
  router->SetMainEvents(GetAppMainEvents(app_id));
  app_routers_.insert(std::make_pair(app_id, router));
}

void ApplicationEventManager::OnAppUnloaded(const std::string& app_id) {
  DCHECK(app_routers_.find(app_id) != app_routers_.end());
  app_routers_.erase(app_id);
}

void ApplicationEventManager::SendEvent(const std::string& app_id,
                                        scoped_refptr<Event> event) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (ApplicationEventRouter* app_router = GetAppRouter(app_id))
    app_router->DispatchEvent(event);
}

void ApplicationEventManager::AttachObserver(const std::string& app_id,
                                             const std::string& event_name,
                                             EventObserver* observer) {
  DCHECK(content::BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (ApplicationEventRouter* app_router = GetAppRouter(app_id))
    app_router->AttachObserver(event_name, observer);
}

void ApplicationEventManager::DetachObserver(const std::string& app_id,
                                             const std::string& event_name,
                                             EventObserver* observer) {
  DCHECK(content::BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (ApplicationEventRouter* app_router = GetAppRouter(app_id))
    app_router->DetachObserver(event_name, observer);
}

void ApplicationEventManager::DetachObserver(EventObserver* observer) {
  DCHECK(content::BrowserThread::CurrentlyOn(BrowserThread::UI));
  AppRouterMap::iterator it = app_routers_.begin();
  for (; it != app_routers_.end(); ++it)
    it->second->DetachObserver(observer);
}

void ApplicationEventManager::OnMainDocumentCreated(
    const std::string& app_id, content::WebContents* contents) {
  if (ApplicationEventRouter* app_router = GetAppRouter(app_id)) {
    DCHECK(app_router);
    app_router->ObserveMainDocument(contents);
  }
}

ApplicationEventRouter* ApplicationEventManager::GetAppRouter(
    const std::string& app_id) {
  AppRouterMap::iterator it = app_routers_.find(app_id);
  if (it != app_routers_.end())
    return it->second.get();

  DLOG(WARNING) << "Application " << app_id << " router not found.";
  return NULL;
}

const std::set<std::string> ApplicationEventManager::GetAppMainEvents(
    const std::string& app_id) {
  NOTIMPLEMENTED();
  return std::set<std::string>();
}

void ApplicationEventManager::SetAppMainEvents(
    const std::string& app_id, const std::set<std::string>& events) {
  NOTIMPLEMENTED();
}

}  // namespace application
}  // namespace xwalk
