// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/permissions_handler.h"

#include "xwalk/application/common/application_manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

namespace {

const PermissionSet& GetAPIPermissionsInfo(
    scoped_refptr<const ApplicationData> application) {
  PermissionsInfo* info = static_cast<PermissionsInfo*>(
      application->GetManifestData(keys::kPermissionsKey));
  DCHECK(info);
  return info->GetAPIPermissions();
}

}  // namespace

class PermissionsHandlerTest: public testing::Test {
};

TEST_F(PermissionsHandlerTest, NonePermission) {
  base::DictionaryValue manifest;
  manifest.SetString(keys::kNameKey, "no name");
  manifest.SetString(keys::kXWalkVersionKey, "0");
  std::string error;
  scoped_refptr<ApplicationData> application = ApplicationData::Create(
      base::FilePath(),
      ApplicationData::LOCAL_DIRECTORY,
      manifest,
      "",
      &error);
  EXPECT_TRUE(application.get());
  EXPECT_EQ(GetAPIPermissionsInfo(application).size(), 0);
}

TEST_F(PermissionsHandlerTest, EmptyPermission) {
  base::DictionaryValue manifest;
  manifest.SetString(keys::kNameKey, "no name");
  manifest.SetString(keys::kXWalkVersionKey, "0");
  base::ListValue* permissions = new base::ListValue;
  manifest.Set(keys::kPermissionsKey, permissions);
  std::string error;
  scoped_refptr<ApplicationData> application = ApplicationData::Create(
      base::FilePath(),
      ApplicationData::LOCAL_DIRECTORY,
      manifest,
      "",
      &error);
  EXPECT_TRUE(application.get());
  EXPECT_EQ(GetAPIPermissionsInfo(application).size(), 0);
}

TEST_F(PermissionsHandlerTest, DeviceAPIPermission) {
  base::DictionaryValue manifest;
  manifest.SetString(keys::kNameKey, "no name");
  manifest.SetString(keys::kXWalkVersionKey, "0");
  base::ListValue* permissions = new base::ListValue;
  permissions->AppendString("geolocation");
  manifest.Set(keys::kPermissionsKey, permissions);
  std::string error;
  scoped_refptr<ApplicationData> application = ApplicationData::Create(
      base::FilePath(),
      ApplicationData::LOCAL_DIRECTORY,
      manifest,
      "",
      &error);
  EXPECT_TRUE(application.get());
  const PermissionSet& permission_list =
      GetAPIPermissionsInfo(application);
  EXPECT_EQ(permission_list.size(), 1);
  EXPECT_STREQ((*(permission_list.begin())).c_str(), "geolocation");
}

}  // namespace application
}  // namespace xwalk
