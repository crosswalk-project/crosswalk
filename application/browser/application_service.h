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
#include "xwalk/application/common/permission_policy_manager.h"
#include "xwalk/application/common/application_data.h"

namespace xwalk {

class XWalkBrowserContext;

namespace application {

// The application service manages launch and termination of the applications.
class ApplicationService : public Application::Observer {
 public:
  // Client code may use this class (and register with AddObserver below) to
  // keep track of applications life cycle.
  class Observer {
   public:
    virtual void DidLaunchApplication(Application* app) {}
    virtual void WillDestroyApplication(Application* app) {}
   protected:
    virtual ~Observer() {}
  };

  virtual ~ApplicationService();

  static scoped_ptr<ApplicationService> Create(
    XWalkBrowserContext* browser_context);

  // Launch an unpacked application using path to the manifest file
  // of an unpacked application.
  Application* LaunchFromManifestPath(
      const base::FilePath& path, Manifest::Type manifest_type,
      const Application::LaunchParams& params = Application::LaunchParams());

  // Launch an application using path to its package file.
  // Note: the given package is unpacked to a temporary folder,
  // which is deleted after the application terminates.
  Application* LaunchFromPackagePath(
      const base::FilePath& path,
      const Application::LaunchParams& params = Application::LaunchParams());

  // Launch an application from an arbitrary URL.
  // Creates a "dummy" application.
  Application* LaunchHostedURL(
      const GURL& url,
      const Application::LaunchParams& params = Application::LaunchParams());

  Application* GetApplicationByRenderHostID(int id) const;
  Application* GetApplicationByID(const std::string& app_id) const;

  const ScopedVector<Application>& active_applications() const {
      return applications_; }

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Check whether application has permission to access API of extension.
  void CheckAPIAccessControl(const std::string& app_id,
      const std::string& extension_name,
      const std::string& api_name, const PermissionCallback& callback);
  // Register APIs implemented by extension. This method will be called
  // when application register extensions.
  // Parameter perm_table is a string which is a map between extension
  // and it includes APIs. For example perm_table is like '{"bluetooth":
  // ["read", "write", "management"]}'.
  bool RegisterPermissions(const std::string& app_id,
      const std::string& extension_name,
      const std::string& perm_table);

 protected:
  explicit ApplicationService(XWalkBrowserContext* browser_context);

  Application* Launch(scoped_refptr<ApplicationData> application_data,
                      const Application::LaunchParams& launch_params);

 private:
  // Implementation of Application::Observer.
  void OnApplicationTerminated(Application* app) override;

  XWalkBrowserContext* browser_context_;
  ScopedVector<Application> applications_;
  ObserverList<Observer> observers_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationService);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
