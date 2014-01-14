// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_

#include <string>
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "base/observer_list.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/application/common/application_data.h"

namespace xwalk {

class RuntimeContext;

namespace application {

class ApplicationStorage;
class ApplicationEventManager;

// The application service manages install, uninstall and updates of
// applications.
class ApplicationService : public Application::Observer {
 public:
  // Client code may use this class (and register with AddObserver below) to
  // keep track of [un]installation of applications.
  class Observer {
   public:
    virtual void OnApplicationInstalled(const std::string& app_id) {}
    virtual void OnApplicationUninstalled(const std::string& app_id) {}

    virtual void DidLaunchApplication(Application* app) {}
    virtual void WillDestroyApplication(Application* app) {}
   protected:
    virtual ~Observer() {}
  };

  ApplicationService(RuntimeContext* runtime_context,
                     ApplicationStorage* app_storage,
                     ApplicationEventManager* event_manager);
  virtual ~ApplicationService();

  bool Install(const base::FilePath& path, std::string* id);
  bool Uninstall(const std::string& id);
  // Launch an installed application using application id.
  Application* Launch(const std::string& id);
  // Launch an unpacked application using path to a local directory which
  // contains manifest file.
  Application* Launch(const base::FilePath& path);
  // Launch an application created from arbitrary url.
  // FIXME: This application should have the same strict permissions
  // as common browser apps.
  Application* Launch(const GURL& url);

  Application* GetApplicationByRenderHostID(int id) const;
  Application* GetApplicationByID(const std::string& app_id) const;

  const ScopedVector<Application>& active_applications() const {
      return applications_; }

  // FIXME: This method should go away when multiple applications
  // running is supported.
  Application* GetActiveApplication() const;

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

 private:
  // Implementation of Application::Observer.
  virtual void OnApplicationTerminated(Application* app) OVERRIDE;

  Application* Launch(scoped_refptr<ApplicationData> application_data,
                      Application::LaunchEntryPoints = Application::Default);

  xwalk::RuntimeContext* runtime_context_;
  ApplicationStorage* application_storage_;
  ApplicationEventManager* event_manager_;
  ScopedVector<Application> applications_;
  ObserverList<Observer> observers_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationService);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
