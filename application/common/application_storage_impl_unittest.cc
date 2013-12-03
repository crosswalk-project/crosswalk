// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_storage_impl.h"

#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_file_value_serializer.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

class ApplicationStorageImplTest : public testing::Test {
 public:
  void TestInit() {
    base::FilePath tmp;
    ASSERT_TRUE(PathService::Get(base::DIR_TEMP, &tmp));
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDirUnderPath(tmp));
    app_storage_impl_.reset(new ApplicationStorageImpl(temp_dir_.path()));
    ASSERT_TRUE(app_storage_impl_->Init(applications));
  }

 protected:
  base::ScopedTempDir temp_dir_;
  scoped_ptr<ApplicationStorageImpl> app_storage_impl_;
  ApplicationData::ApplicationDataMap applications;
};

TEST_F(ApplicationStorageImplTest, CreateDBFile) {
  TestInit();
  EXPECT_TRUE(base::PathExists(
      temp_dir_.path().Append(ApplicationStorageImpl::kDBFileName)));
}

TEST_F(ApplicationStorageImplTest, DBInsert) {
  TestInit();
  base::DictionaryValue manifest;
  manifest.SetString(keys::kNameKey, "no name");
  manifest.SetString(keys::kVersionKey, "0");
  manifest.SetString("a", "b");
  std::string error;
  scoped_refptr<ApplicationData> application = ApplicationData::Create(
      base::FilePath(),
      Manifest::INVALID_TYPE,
      manifest,
      "",
      &error);
  ASSERT_TRUE(error.empty());
  ASSERT_TRUE(application);
  std::set<std::string> events;
  events.insert("test_events");
  application->SetEvents(events);
  EXPECT_TRUE(app_storage_impl_->AddApplication(application.get(),
                                                base::Time::FromDoubleT(0)));
  ApplicationData::ApplicationDataMap applications;
  ASSERT_TRUE(app_storage_impl_->GetInstalledApplications(applications));
  EXPECT_EQ(applications.size(), 1);
  EXPECT_TRUE(applications[application->ID()]);
  EXPECT_EQ(applications[application->ID()]
            ->GetEvents().count("test_events"), 1);
}

TEST_F(ApplicationStorageImplTest, DBDelete) {
  TestInit();
  base::DictionaryValue manifest;
  manifest.SetString(keys::kNameKey, "no name");
  manifest.SetString(keys::kVersionKey, "0");
  manifest.SetString("a", "b");
  std::string error;
  scoped_refptr<ApplicationData> application =
      ApplicationData::Create(base::FilePath(),
                              Manifest::INTERNAL,
                              manifest,
                              "",
                              &error);
  ASSERT_TRUE(error.empty());
  EXPECT_TRUE(application);
  EXPECT_TRUE(app_storage_impl_->AddApplication(application.get(),
                                                base::Time::FromDoubleT(0)));

  EXPECT_TRUE(app_storage_impl_->RemoveApplication(application->ID()));
  ApplicationData::ApplicationDataMap applications;
  ASSERT_TRUE(app_storage_impl_->GetInstalledApplications(applications));
  EXPECT_EQ(applications.size(), 0);
}

TEST_F(ApplicationStorageImplTest, DBUpgradeToV1) {
  base::FilePath tmp;
  ASSERT_TRUE(PathService::Get(base::DIR_TEMP, &tmp));
  ASSERT_TRUE(temp_dir_.CreateUniqueTempDirUnderPath(tmp));

  scoped_ptr<base::DictionaryValue> db_value(new base::DictionaryValue);
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  value->SetString("path", "");
  base::DictionaryValue* manifest = new base::DictionaryValue;
  manifest->SetString("a", "b");
  manifest->SetString(keys::kNameKey, "no name");
  manifest->SetString(keys::kVersionKey, "0");
  value->Set("manifest", manifest);
  value->SetDouble("install_time", 0);
  db_value->Set("test_id", value.release());

  base::FilePath v0_db_file(
      temp_dir_.path().AppendASCII("applications_db"));
  JSONFileValueSerializer serializer(v0_db_file);
  ASSERT_TRUE(serializer.Serialize(*db_value.get()));
  ASSERT_TRUE(base::PathExists(v0_db_file));

  app_storage_impl_.reset(new ApplicationStorageImpl(temp_dir_.path()));
  ApplicationData::ApplicationDataMap applications;
  ASSERT_TRUE(app_storage_impl_->Init(applications));
  ASSERT_FALSE(base::PathExists(v0_db_file));
  EXPECT_EQ(applications.size(), 1);
  EXPECT_TRUE(applications["test_id"]);
}

TEST_F(ApplicationStorageImplTest, DBUpdate) {
  TestInit();
  base::DictionaryValue manifest;
  manifest.SetString(keys::kNameKey, "no name");
  manifest.SetString(keys::kVersionKey, "0");
  manifest.SetString("a", "b");
  std::string error;
  scoped_refptr<ApplicationData> application =
      ApplicationData::Create(base::FilePath(),
                              Manifest::INTERNAL,
                              manifest,
                              "",
                              &error);
  ASSERT_TRUE(error.empty());
  ASSERT_TRUE(application);
  EXPECT_TRUE(app_storage_impl_->AddApplication(application.get(),
                                                base::Time::FromDoubleT(0)));

  manifest.SetString("a", "c");
  scoped_refptr<ApplicationData> new_application =
      ApplicationData::Create(base::FilePath(),
                              Manifest::INTERNAL,
                              manifest,
                              "",
                              &error);
  ASSERT_TRUE(error.empty());
  ASSERT_TRUE(new_application);
  EXPECT_TRUE(app_storage_impl_->UpdateApplication(new_application.get(),
                                                   base::Time::FromDoubleT(0)));
  ApplicationData::ApplicationDataMap applications;
  ASSERT_TRUE(app_storage_impl_->GetInstalledApplications(applications));
  EXPECT_EQ(applications.size(), 1);
  scoped_refptr<ApplicationData> saved_application =
      applications[new_application->ID()];
  EXPECT_TRUE(saved_application);

  EXPECT_TRUE(saved_application->GetManifest()->value()->Equals(
      new_application->GetManifest()->value()));
}

}  // namespace application
}  // namespace xwalk
