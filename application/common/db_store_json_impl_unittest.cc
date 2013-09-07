// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/db_store_json_impl.h"

#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_file_value_serializer.h"
#include "base/path_service.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/common/application_file_util.h"

namespace xwalk {
namespace application {

class DBStoreJsonImplTest : public testing::Test {
 public:
  virtual ~DBStoreJsonImplTest() {
    temp_dir_.Delete();
  }

  void SetDB(const std::string& db_dir) {
    base::FilePath db_path;
    ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &db_path));
    db_path = db_path.AppendASCII("xwalk")
        .AppendASCII("application")
        .AppendASCII("test")
        .AppendASCII("db")
        .AppendASCII(db_dir);

    base::FilePath tmp;
    ASSERT_TRUE(PathService::Get(base::DIR_TEMP, &tmp));
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDirUnderPath(tmp));
    db_path_ = temp_dir_.path().AppendASCII(db_dir);
    ASSERT_TRUE(file_util::CopyDirectory(db_path, db_path_, true));
    db_store_.reset(new DBStoreJsonImpl(db_path_));
  }

 protected:
  base::ScopedTempDir temp_dir_;
  scoped_ptr<DBStoreJsonImpl> db_store_;
  base::FilePath db_path_;
};

TEST_F(DBStoreJsonImplTest, ReadDataBase) {
  SetDB("good");
  EXPECT_TRUE(db_store_->InitDB());
  JSONFileValueSerializer serializer(db_path_.AppendASCII("applications_db"));
  int error_code;
  std::string error_msg;
  scoped_ptr<base::Value> value(
      serializer.Deserialize(&error_code, &error_msg));
  EXPECT_TRUE(db_store_->GetApplications()->Equals(value.get()));
}

}  // namespace application
}  // namespace xwalk
