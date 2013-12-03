// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_

#include <string>
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "xwalk/application/browser/application_store.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/application/common/application.h"

namespace xwalk {
class RuntimeContext;
}

namespace xwalk {
namespace application {

// This will manages applications install, uninstall, update and so on. It'll
// also maintain all installed applications' info.
class ApplicationService {
 public:
  explicit ApplicationService(xwalk::RuntimeContext* runtime_context);
  virtual ~ApplicationService();

  bool Install(const base::FilePath& path, std::string* id);
  bool Uninstall(const std::string& id);
  bool Launch(const std::string& id);
  bool Launch(const base::FilePath& path);

  scoped_refptr<const Application> GetApplicationByID(
       const std::string& id) const;
  ApplicationStore::ApplicationMap* GetInstalledApplications() const;
  // Currently there's only one running application at a time.
  const Application* GetRunningApplication() const;
  // Check whether application has the permisson to access API. This API will
  // be called by XWalkExtensionProcessHost.
  static bool CheckAPIAccessControl(std::string extension_name,
      std::string app_id, std::string api_name);

  // Client code may use this class (and register with AddObserver below) to
  // keep track of applications installed/uninstalled.
  struct Observer {
   public:
    virtual void OnApplicationInstalled(const std::string& app_id) {}
    virtual void OnApplicationUninstalled(const std::string& app_id) {}
   protected:
    ~Observer() {}
  };

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

 private:
  xwalk::RuntimeContext* runtime_context_;
  scoped_ptr<ApplicationStore> app_store_;
  scoped_refptr<const Application> application_;
  ObserverList<Observer> observers_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationService);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
