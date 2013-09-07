// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_DB_STORE_JSON_IMPL_H_
#define XWALK_APPLICATION_COMMON_DB_STORE_JSON_IMPL_H_

#include <string>

#include "base/files/important_file_writer.h"
#include "base/sequenced_task_runner.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/values.h"
#include "xwalk/application/common/db_store.h"

namespace xwalk {
namespace application {

// The JSON backend implementation of DBStore.
class DBStoreJsonImpl: public DBStore,
    public base::ImportantFileWriter::DataSerializer {
 public:
  explicit DBStoreJsonImpl(base::FilePath path);
  ~DBStoreJsonImpl();
  // Returns instance of SequencedTaskRunner which guarantees that file
  // operations on the same file will be executed in sequenced order.
  static scoped_refptr<base::SequencedTaskRunner> GetTaskRunnerForFile(
      const base::FilePath& db_filename,
      base::SequencedWorkerPool* worker_pool);

  // Implement the DBStore interface.
  virtual bool Insert(const Application* application,
                      const base::Time install_time) OVERRIDE;

  virtual bool InitDB() OVERRIDE;
  virtual void SetValue(const std::string& key, base::Value* value) OVERRIDE;

  void OnFileRead(base::Value* value_owned, std::string error_msg, bool no_dir);

 private:
  // ImportantFileWriter::DataSerializer overrides:
  virtual bool SerializeData(std::string* output) OVERRIDE;

  void ReportValueChanged(const std::string& key, const base::Value* value);
  void CommitPendingWrite();
  // Helper for safely writing db
  scoped_ptr<base::ImportantFileWriter> writer_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
};

}  // namespace application
}  // namespace xwalk
#endif  // XWALK_APPLICATION_COMMON_DB_STORE_JSON_IMPL_H_
