// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/runtime_registry.h"

#include "cameo/src/runtime/browser/runtime.h"
#include "cameo/src/runtime/common/cameo_notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

using content::RenderViewHost;

namespace cameo {

// An application-wide runtime registry.
static RuntimeRegistry* g_runtime_registry = NULL;

RuntimeRegistry::RuntimeRegistry() {
  DCHECK(g_runtime_registry == NULL);
  g_runtime_registry = this;
}

RuntimeRegistry::~RuntimeRegistry() {
  DCHECK(g_runtime_registry);
  DCHECK(runtime_list_.empty()) <<
      "Runtime instances are not empty!";
  g_runtime_registry = NULL;
}

// static
RuntimeRegistry* RuntimeRegistry::Get() {
  return g_runtime_registry;
}

void RuntimeRegistry::AddObserver(RuntimeRegistryObserver* obs) {
  observer_list_.AddObserver(obs);
}

void RuntimeRegistry::RemoveObserver(RuntimeRegistryObserver* obs) {
  observer_list_.RemoveObserver(obs);
}

void RuntimeRegistry::AddRuntime(Runtime* runtime) {
  runtime_list_.push_back(runtime);

  content::NotificationService::current()->Notify(
      cameo::NOTIFICATION_RUNTIME_OPENED,
      content::Source<Runtime>(runtime),
      content::NotificationService::NoDetails());

  FOR_EACH_OBSERVER(RuntimeRegistryObserver, observer_list_,
      OnRuntimeAdded(runtime));
}

void RuntimeRegistry::RemoveRuntime(Runtime* runtime) {
  RuntimeList::iterator it =
      std::find(runtime_list_.begin(), runtime_list_.end(), runtime);
  if (it != runtime_list_.end())
    runtime_list_.erase(it);

  content::NotificationService::current()->Notify(
      cameo::NOTIFICATION_RUNTIME_CLOSED,
      content::Source<Runtime>(runtime),
      content::NotificationService::NoDetails());

  FOR_EACH_OBSERVER(RuntimeRegistryObserver, observer_list_,
                    OnRuntimeRemoved(runtime));
}

void RuntimeRegistry::RuntimeAppIconChanged(Runtime* runtime) {
  RuntimeList::iterator it =
      std::find(runtime_list_.begin(), runtime_list_.end(), runtime);
  DCHECK(it != runtime_list_.end());

  FOR_EACH_OBSERVER(RuntimeRegistryObserver, observer_list_,
                    OnRuntimeAppIconChanged(runtime));
}

Runtime* RuntimeRegistry::GetRuntimeFromRenderViewHost(
    RenderViewHost* render_view_host) const {
  for (RuntimeList::const_iterator it = runtime_list_.begin();
       it != runtime_list_.end(); ++it) {
    if ((*it)->web_contents()->GetRenderViewHost() == render_view_host)
      return (*it);
  }
  return NULL;
}

void RuntimeRegistry::CloseAll() {
  RuntimeList cached_runtimes;

  RuntimeList::iterator it = runtime_list_.begin();
  for (; it != runtime_list_.end(); ++it) {
    cached_runtimes.push_back(*it);
  }

  for (it = cached_runtimes.begin(); it != cached_runtimes.end(); ++it)
    (*it)->Close();

  // If a Runtime is closed, it will be deleted by itself and also be removed
  // from RuntimeRegistry. The runtime vector should be empty after all
  // Runtime instances are closed.
  DCHECK_EQ(runtime_list_.size(), 0u) << runtime_list_.size();
}

}  // namespace cameo
