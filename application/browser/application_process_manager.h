// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "xwalk/application/common/application.h"
#include "xwalk/runtime/browser/runtime_registry.h"

class GURL;

namespace xwalk {
class Runtime;
class RuntimeContext;
}

namespace xwalk {
namespace application {

class ApplicationHost;
class Manifest;

// This manages dynamic state of running applications. By now, it only launches
// one application, later it will manages all event pages' lifecycle.
class ApplicationProcessManager : public RuntimeRegistryObserver {
 public:
  explicit ApplicationProcessManager(xwalk::RuntimeContext* runtime_context);
  virtual ~ApplicationProcessManager();

  bool LaunchApplication(xwalk::RuntimeContext* runtime_context,
                         const Application* application);

  Runtime* GetMainDocumentRuntime() const { return main_runtime_; }

  // RuntimeRegistryObserver implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeAppIconChanged(Runtime* runtime) OVERRIDE {}

 private:
  bool RunMainDocument(const Application* application);
  bool RunFromLocalPath(const Application* application);
  void CloseMainDocument();

  xwalk::RuntimeContext* runtime_context_;
  xwalk::Runtime* main_runtime_;
  base::WeakPtrFactory<ApplicationProcessManager> weak_ptr_factory_;
  std::set<Runtime*> runtimes_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationProcessManager);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_
