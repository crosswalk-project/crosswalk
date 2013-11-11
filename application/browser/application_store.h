// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_STORE_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_STORE_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/db_store_sqlite_impl.h"

namespace xwalk {
class Runtime;
class RuntimeContext;
}

namespace xwalk {
namespace application {

class ApplicationStore: public DBStore::Observer {
 public:
  typedef DBStoreSqliteImpl DBStoreImpl;
  typedef std::map<std::string, scoped_refptr<const Application> >
      ApplicationMap;
  typedef std::map<std::string, scoped_refptr<const Application> >::iterator
      ApplicationMapIterator;

  // The constaints for application storage.
  static const char kManifestPath[];
  static const char kApplicationPath[];
  static const char kInstallTime[];
  static const char kRegisteredEvents[];

  explicit ApplicationStore(xwalk::RuntimeContext* runtime_context);
  virtual ~ApplicationStore();

  bool AddApplication(scoped_refptr<const Application> application);

  bool RemoveApplication(const std::string& id);

  bool Contains(const std::string& app_id) const;

  scoped_refptr<const Application> GetApplicationByID(
      const std::string& application_id) const;

  ApplicationMap* GetInstalledApplications() const;

  bool SetApplicationEvents(const std::string& id,
                            base::ListValue* events);
  base::ListValue* GetApplicationEvents(const std::string& id);

  // Implement the DBStore::Observer.
  virtual void OnDBValueChanged(const std::string& key,
                                const base::Value* value) OVERRIDE;
  virtual void OnInitializationCompleted(bool succeeded) OVERRIDE;

 private:
  void InitApplications(const base::DictionaryValue* value);
  bool Insert(scoped_refptr<const Application> application);
  xwalk::RuntimeContext* runtime_context_;
  scoped_ptr<DBStoreImpl> db_store_;
  scoped_ptr<ApplicationMap> applications_;
  DISALLOW_COPY_AND_ASSIGN(ApplicationStore);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_STORE_H_
