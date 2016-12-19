// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "base/files/file_path.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/extensions/common/xwalk_external_extension.h"
#include "xwalk/extensions/common/xwalk_external_instance.h"

using namespace xwalk::extensions;  // NOLINT

class TestExtension : public XWalkExternalExtension {
 public:
  explicit TestExtension(const base::FilePath& path)
    : XWalkExternalExtension(path),
    runtime_variables_(new base::DictionaryValue::Storage) {
    (*runtime_variables_)["extension_path"] =
        base::WrapUnique(new base::StringValue(path.AsUTF8Unsafe()));
    set_runtime_variables(runtime_variables_.get());
  }
  XWalkExternalInstance* CreateExternalInstance() {
    return static_cast<XWalkExternalInstance*>(
        XWalkExternalExtension::CreateInstance());
  }
 private:
  std::unique_ptr<base::DictionaryValue::Storage> runtime_variables_;
};

TEST(XWalkDotNetExtensionTest, InvalidExtensions) {
  base::FilePath test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("echo_extension/echo_extension_bridge.dll"));
  TestExtension* valid_extension = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(valid_extension->Initialize());
  XWalkExternalInstance* instance = valid_extension->CreateExternalInstance();
  EXPECT_TRUE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_1/invalid_extension_1_bridge.dll"));
  TestExtension* invalid_extension_1 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_1->Initialize());
  instance = invalid_extension_1->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_2/invalid_extension_2_bridge.dll"));
  TestExtension* invalid_extension_2 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_2->Initialize());
  instance = invalid_extension_2->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_3/invalid_extension_3_bridge.dll"));
  TestExtension* invalid_extension_3 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_3->Initialize());
  instance = invalid_extension_3->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_4/invalid_extension_4_bridge.dll"));
  TestExtension* invalid_extension_4 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_4->Initialize());
  instance = invalid_extension_4->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_5/invalid_extension_5_bridge.dll"));
  TestExtension* invalid_extension_5 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_5->Initialize());
  instance = invalid_extension_5->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_6/invalid_extension_6_bridge.dll"));
  TestExtension* invalid_extension_6 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_6->Initialize());
  instance = invalid_extension_6->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_7/invalid_extension_7_bridge.dll"));
  TestExtension* invalid_extension_7 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_7->Initialize());
  instance = invalid_extension_7->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_8/invalid_extension_8_bridge.dll"));
  TestExtension* invalid_extension_8 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_8->Initialize());
  instance = invalid_extension_8->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_9/invalid_extension_9_bridge.dll"));
  TestExtension* invalid_extension_9 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_9->Initialize());
  instance = invalid_extension_9->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_10/invalid_extension_10_bridge.dll"));
  TestExtension* invalid_extension_10 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_10->Initialize());
  instance = invalid_extension_10->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_11/invalid_extension_11_bridge.dll"));
  TestExtension* invalid_extension_11 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_11->Initialize());
  instance = invalid_extension_11->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_12/invalid_extension_12_bridge.dll"));
  TestExtension* invalid_extension_12 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_12->Initialize());
  instance = invalid_extension_12->CreateExternalInstance();
  EXPECT_FALSE(instance->GetInstanceData());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("binary_extension/binary_extension_bridge.dll"));
  TestExtension* binary_extension = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(binary_extension->Initialize());
  instance = binary_extension->CreateExternalInstance();
  EXPECT_TRUE(instance->GetInstanceData());
}
