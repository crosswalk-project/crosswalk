// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_RUNTIME_BROWSER_RUNTIME_REGISTRY_H_
#define CAMEO_RUNTIME_BROWSER_RUNTIME_REGISTRY_H_

#include <vector>

#include "base/observer_list.h"

namespace content {
class RenderViewHost;
};

namespace xwalk {

class Runtime;

// The observer for runtime registry changes.
class RuntimeRegistryObserver {
 public:
  // Called when a new Runtime instance is added.
  virtual void OnRuntimeAdded(Runtime* runtime) = 0;

  // Called when a Runtime instance is removed.
  virtual void OnRuntimeRemoved(Runtime* runtime) = 0;

  // Called when Runtime's app icon is changed.
  virtual void OnRuntimeAppIconChanged(Runtime* runtime) = 0;

 protected:
  virtual ~RuntimeRegistryObserver() {}
};

typedef std::vector<Runtime*> RuntimeList;

// RuntimeRegistry maintains a list of Runtime created for running app.
// It allows to retrieve all Runtime instances via RuntimeRegistry.
class RuntimeRegistry {
 public:
  // Get the singleton instance of RuntimeRegistry.
  static RuntimeRegistry* Get();

  RuntimeRegistry();
  ~RuntimeRegistry();

  // Add/remove runtime.
  void AddRuntime(Runtime* runtime);
  void RemoveRuntime(Runtime* runtime);

  void RuntimeAppIconChanged(Runtime* runtime);

  // Find a runtime from a RenderViewHost
  Runtime* GetRuntimeFromRenderViewHost(
      content::RenderViewHost* render_view_host) const;
  const RuntimeList& runtimes() const { return runtime_list_; }

  // Close all running Runtime instances.
  void CloseAll();

  // Add/remove observer.
  void AddObserver(RuntimeRegistryObserver* obs);
  void RemoveObserver(RuntimeRegistryObserver* obs);

 private:
  RuntimeList runtime_list_;

  ObserverList<RuntimeRegistryObserver> observer_list_;
};

}  // namespace xwalk

#endif  // CAMEO_RUNTIME_BROWSER_RUNTIME_REGISTRY_H_
