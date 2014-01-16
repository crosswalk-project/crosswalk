// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_event_manager.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/application/browser/application_event_router.h"
#include "xwalk/application/browser/application.h"

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

ApplicationEventManager::ApplicationEventManager() {
}

ApplicationEventManager::~ApplicationEventManager() {
}

void ApplicationEventManager::AddEventRouterForApp(
    scoped_refptr<ApplicationData> app_data) {
  std::set<std::string> events = app_data->GetEvents();

  linked_ptr<ApplicationEventRouter> router(
      new ApplicationEventRouter(app_data->ID()));
  router->SetMainEvents(events);

  app_routers_.insert(std::make_pair(app_data->ID(), router));
}

void ApplicationEventManager::RemoveEventRouterForApp(
    scoped_refptr<ApplicationData> app_data) {
  DCHECK(app_routers_.find(app_data->ID()) != app_routers_.end());
  app_routers_.erase(app_data->ID());
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

void ApplicationEventManager::DidLaunchApplication(Application* app) {
  if (Runtime* runtime = app->GetMainDocumentRuntime()) {
    ApplicationEventRouter* app_router = GetAppRouter(app->id());
    CHECK(app_router);
    app_router->ObserveMainDocument(runtime->web_contents());
  }
}

void ApplicationEventManager::WillDestroyApplication(Application* app) {
  RemoveEventRouterForApp(app->data());
}

ApplicationEventRouter* ApplicationEventManager::GetAppRouter(
    const std::string& app_id) {
  AppRouterMap::iterator it = app_routers_.find(app_id);
  if (it != app_routers_.end())
    return it->second.get();

  DLOG(WARNING) << "Application " << app_id << " router not found.";
  return NULL;
}

}  // namespace application
}  // namespace xwalk
