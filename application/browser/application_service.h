// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_

#include <string>
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/application/common/application_data.h"

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

  scoped_refptr<ApplicationData> GetApplicationByID(
       const std::string& id) const;
  const ApplicationData::ApplicationDataMap& GetInstalledApplications() const;
  // Currently there's only one running application at a time.
  const ApplicationData* GetRunningApplication() const;

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
  ApplicationStorage* application_storage();

 private:
  bool Launch(scoped_refptr<const ApplicationData> application);
  bool GenerateAppIDAndLoadApplication(
      const base::FilePath& package_path, const base::FilePath& data_dir,
      std::string& app_id, scoped_refptr<ApplicationData>& application);

  xwalk::RuntimeContext* runtime_context_;
  scoped_ptr<ApplicationStorage> app_storage_;
  scoped_refptr<const ApplicationData> application_;
  ObserverList<Observer> observers_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationService);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
