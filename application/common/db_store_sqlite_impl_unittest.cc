// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/db_store_sqlite_impl.h"

#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_file_value_serializer.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/browser/application_store.h"

namespace xwalk {
namespace application {

class DBStoreSqliteImplTest : public testing::Test {
 public:
  void TestInit() {
    base::FilePath tmp;
    ASSERT_TRUE(PathService::Get(base::DIR_TEMP, &tmp));
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDirUnderPath(tmp));
    db_store_.reset(new DBStoreSqliteImpl(temp_dir_.path()));
    ASSERT_TRUE(db_store_->InitDB());
  }

 protected:
  base::ScopedTempDir temp_dir_;
  scoped_ptr<DBStoreSqliteImpl> db_store_;
};

TEST_F(DBStoreSqliteImplTest, CreateDBFile) {
  TestInit();
  EXPECT_TRUE(base::PathExists(
      temp_dir_.path().Append(DBStoreSqliteImpl::kDBFileName)));
}

TEST_F(DBStoreSqliteImplTest, DBInsert) {
  TestInit();
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  value->SetString(ApplicationStore::kApplicationPath, "no path");
  base::DictionaryValue* manifest = new base::DictionaryValue;
  manifest->SetString("a", "b");
  value->Set(ApplicationStore::kManifestPath, manifest);
  value->SetDouble(ApplicationStore::kInstallTime, 0);
  std::string id("test_id");
  db_store_->SetValue(id, value.release());

  scoped_ptr<base::DictionaryValue> old_value(
      db_store_->GetApplications()->DeepCopy());
  ASSERT_TRUE(old_value->HasKey(id));
  // Refresh the database cache from db file.
  ASSERT_TRUE(db_store_->InitDB());
  EXPECT_TRUE(old_value->Equals(db_store_->GetApplications()));
}

TEST_F(DBStoreSqliteImplTest, DBDelete) {
  TestInit();
  scoped_ptr<base::DictionaryValue> old_value(
      db_store_->GetApplications()->DeepCopy());
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  value->SetString(ApplicationStore::kApplicationPath, "no path");
  base::DictionaryValue* manifest = new base::DictionaryValue;
  manifest->SetString("a", "b");
  value->Set(ApplicationStore::kManifestPath, manifest);
  value->SetDouble(ApplicationStore::kInstallTime, 0);
  std::string id("test_id");
  db_store_->SetValue(id, value.release());

  ASSERT_TRUE(db_store_->Remove(id));
  // Refresh the database cache from db file.
  ASSERT_TRUE(db_store_->InitDB());
  EXPECT_TRUE(old_value->Equals(db_store_->GetApplications()));
}

TEST_F(DBStoreSqliteImplTest, DBUpgradeToV1) {
  base::FilePath tmp;
  ASSERT_TRUE(PathService::Get(base::DIR_TEMP, &tmp));
  ASSERT_TRUE(temp_dir_.CreateUniqueTempDirUnderPath(tmp));

  scoped_ptr<base::DictionaryValue> db_value(new base::DictionaryValue);
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  value->SetString(ApplicationStore::kApplicationPath, "path");
  base::DictionaryValue* manifest = new base::DictionaryValue;
  manifest->SetString("a", "b");
  value->Set(ApplicationStore::kManifestPath, manifest);
  value->SetDouble(ApplicationStore::kInstallTime, 0);
  db_value->Set("test_id", value.release());

  base::FilePath v0_db_file(
      temp_dir_.path().AppendASCII("applications_db"));
  JSONFileValueSerializer serializer(v0_db_file);
  ASSERT_TRUE(serializer.Serialize(*db_value.get()));
  ASSERT_TRUE(base::PathExists(v0_db_file));

  db_store_.reset(new DBStoreSqliteImpl(temp_dir_.path()));
  ASSERT_TRUE(db_store_->InitDB());
  ASSERT_FALSE(base::PathExists(v0_db_file));
  EXPECT_TRUE(db_value->Equals(db_store_->GetApplications()));
}

TEST_F(DBStoreSqliteImplTest, DBUpdate1) {
  TestInit();
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  value->SetString(ApplicationStore::kApplicationPath, "path1");
  base::DictionaryValue* manifest = new base::DictionaryValue;
  manifest->SetString("a", "b");
  value->Set(ApplicationStore::kManifestPath, manifest);
  value->SetDouble(ApplicationStore::kInstallTime, 0);
  std::string id("test_id");
  db_store_->SetValue(id, value.release());

  db_store_->SetValue(id+"."+ApplicationStore::kApplicationPath,
                      base::Value::CreateStringValue("path2"));

  ASSERT_TRUE(db_store_->InitDB());
  std::string path;
  ASSERT_TRUE(db_store_->GetApplications()->GetString(
      id+"."+ApplicationStore::kApplicationPath,
      &path));
  EXPECT_EQ(path.compare("path2"), 0);
}

TEST_F(DBStoreSqliteImplTest, DBUpdate2) {
  TestInit();
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  value->SetString(ApplicationStore::kApplicationPath, "path");
  base::DictionaryValue* manifest = new base::DictionaryValue;
  manifest->SetString("a", "b");
  value->Set(ApplicationStore::kManifestPath, manifest);
  value->SetDouble(ApplicationStore::kInstallTime, 0);
  std::string id("test_id");
  db_store_->SetValue(id, value.release());

  db_store_->SetValue(id+"."+ApplicationStore::kInstallTime,
                      base::Value::CreateDoubleValue(1));

  ASSERT_TRUE(db_store_->InitDB());
  double time;
  ASSERT_TRUE(db_store_->GetApplications()->GetDouble(
      id+"."+ApplicationStore::kInstallTime,
      &time));
  EXPECT_EQ(time, 1);
}

TEST_F(DBStoreSqliteImplTest, DBUpdate3) {
  TestInit();
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  value->SetString(ApplicationStore::kApplicationPath, "path");
  base::DictionaryValue* manifest = new base::DictionaryValue;
  manifest->SetString("a", "b");
  value->Set(ApplicationStore::kManifestPath, manifest);
  value->SetDouble(ApplicationStore::kInstallTime, 0);
  std::string id("test_id");
  db_store_->SetValue(id, value.release());

  base::DictionaryValue* new_manifest = manifest->DeepCopy();
  new_manifest->SetString("a", "c");
  db_store_->SetValue(id+"."+ApplicationStore::kManifestPath,
                      new_manifest);

  ASSERT_TRUE(db_store_->InitDB());
  std::string v;
  ASSERT_TRUE(db_store_->GetApplications()->GetString(
      id+"."+ApplicationStore::kManifestPath+".a",
      &v));
  EXPECT_EQ(v.compare("c"), 0);
}

TEST_F(DBStoreSqliteImplTest, DBUpdate4) {
  TestInit();
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  value->SetString(ApplicationStore::kApplicationPath, "path1");
  base::DictionaryValue* manifest = new base::DictionaryValue;
  manifest->SetString("a", "b");
  value->Set(ApplicationStore::kManifestPath, manifest);
  value->SetDouble(ApplicationStore::kInstallTime, 0);
  std::string id("test_id");
  scoped_ptr<base::DictionaryValue> new_value(value->DeepCopy());
  db_store_->SetValue(id, value.release());

  new_value->SetString(ApplicationStore::kApplicationPath, "path2");
  base::DictionaryValue* new_manifest = manifest->DeepCopy();
  new_manifest->SetString("a", "c");
  new_value->Set(ApplicationStore::kManifestPath, new_manifest);
  new_value->SetDouble(ApplicationStore::kInstallTime, 1);
  scoped_ptr<base::DictionaryValue> changed_value(new_value->DeepCopy());
  db_store_->SetValue(id, new_value.release());

  ASSERT_TRUE(db_store_->InitDB());
  scoped_ptr<base::DictionaryValue> db_cache(
      db_store_->GetApplications()->DeepCopy());
  base::DictionaryValue* db_value;
  ASSERT_TRUE(db_cache->GetDictionary(id, &db_value));
  EXPECT_TRUE(changed_value->Equals(db_value));
}

}  // namespace application
}  // namespace xwalk
