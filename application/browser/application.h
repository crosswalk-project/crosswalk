// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "xwalk/application/browser/event_observer.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/runtime/browser/runtime.h"

class GURL;

namespace xwalk {
class RuntimeContext;
}

namespace xwalk {
namespace application {

class ApplicationHost;
class Manifest;

// The Application class is representing an active (running) application.
// Application instances are owned by ApplicationService.
// ApplicationService will delete an Application instance when it is
// terminated.
// FIXME(Mikhail): now application just quits main loop when it is closed, the
// behavior will be like the described above when running of multiple apps
// is supported.
// FIXME(Mikhail): Add information about the relationship of the Application
// and the render process.
class Application : public Runtime::Observer {
 public:
  ~Application();
  // Returns the unique application id which is used to distinguish the
  // application amoung both running applications and installed ones
  // (ApplicationData objects).
  std::string id() const { return application_data_->ID(); }

  // Closes all the application's runtimes (application pages).
  void Close();

  // Returns Runtime (application page) containing the application's
  // 'main document'. The main document is the main entry point of
  // the application to the system. This method will return 'NULL'
  // if application has different entry point (local path manifest key).
  // See http://anssiko.github.io/runtime/app-lifecycle.html#dfn-main-document
  // The main document never has a visible window on its own.
  Runtime* GetMainDocumentRuntime() const { return main_runtime_; }

  const std::set<Runtime*>& runtimes() const { return runtimes_; }

  const ApplicationData* data() const { return application_data_; }

 private:
  // Runtime::Observer implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE;

  // We enforce ApplicationService ownership.
  friend class ApplicationService;
  Application(scoped_refptr<const ApplicationData> data,
              xwalk::RuntimeContext* context);
  bool Launch();
  bool is_launched() const { return !runtimes_.empty(); }

  friend class FinishEventObserver;
  bool RunMainDocument();
  bool RunFromLocalPath();
  void CloseMainDocument();
  bool IsOnSuspendHandlerRegistered(const std::string& app_id) const;

  xwalk::RuntimeContext* runtime_context_;
  scoped_refptr<const ApplicationData> application_data_;
  xwalk::Runtime* main_runtime_;
  base::WeakPtrFactory<Application> weak_ptr_factory_;
  std::set<Runtime*> runtimes_;
  scoped_ptr<EventObserver> finish_observer_;

  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_H_
