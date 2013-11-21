// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_DB_STORE_H_
#define XWALK_APPLICATION_COMMON_DB_STORE_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/time/time.h"
#include "xwalk/application/common/application.h"

namespace xwalk {
namespace application {

class DBStore {
 public:
  // Observer interface for monitoring DBStore.
  class Observer {
   public:
    // Called when the value for the given |key| in the store changes.
    virtual void OnDBValueChanged(const std::string& key,
                                  const base::Value* value) = 0;
    // Notification about the DBStore being fully initialized.
    virtual void OnInitializationCompleted(bool succeeded) = 0;

   protected:
    virtual ~Observer() {}
  };
  explicit DBStore(base::FilePath path);
  virtual ~DBStore();
  virtual bool Insert(const Application* application,
                      const base::Time install_time) = 0;
  virtual bool Remove(const std::string& key) = 0;
  base::DictionaryValue* GetApplications() const { return db_.get(); }

  void AddObserver(DBStore::Observer* observer) {
    observers_.AddObserver(observer);
  }
  void RemoveObserver(DBStore::Observer* observer) {
    observers_.RemoveObserver(observer);
  }

  // Initialize the database, calling OnInitializationCompleted for each
  // observer on completion.
  virtual bool InitDB() = 0;

  // Set value in database, resulting is calling OnDBValueChanged for
  // each observer.
  virtual void SetValue(const std::string& key, base::Value* value) = 0;

 protected:
  scoped_ptr<base::DictionaryValue> db_;
  base::FilePath data_path_;
  ObserverList<DBStore::Observer, true> observers_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_DB_STORE_H_
