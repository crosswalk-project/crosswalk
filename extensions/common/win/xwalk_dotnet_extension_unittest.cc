// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/win/xwalk_dotnet_extension.h"
#include "xwalk/extensions/common/win/xwalk_dotnet_instance.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "testing/gtest/include/gtest/gtest.h"

using namespace xwalk::extensions;  // NOLINT

class TestExtension : public XWalkDotNetExtension {
 public:
  explicit TestExtension(const base::FilePath& path)
    : XWalkDotNetExtension(path) {
  }
  XWalkDotNetInstance* CreateDotNetInstance() {
    return static_cast<XWalkDotNetInstance*>(CreateInstance());
  }
};

TEST(XWalkDotNetExtensionTest, InvalidExtensions) {
  base::FilePath test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("echo_extension/echo_extension.dll"));
  TestExtension* valid_extension = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(valid_extension->Initialize());
  EXPECT_TRUE(valid_extension->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_1/invalid_extension_1.dll"));
  TestExtension* invalid_extension_1 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_1->Initialize());
  EXPECT_FALSE(invalid_extension_1->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_2/invalid_extension_2.dll"));
  TestExtension* invalid_extension_2 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_2->Initialize());
  EXPECT_FALSE(invalid_extension_2->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_3/invalid_extension_3.dll"));
  TestExtension* invalid_extension_3 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_3->Initialize());
  EXPECT_FALSE(invalid_extension_3->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_4/invalid_extension_4.dll"));
  TestExtension* invalid_extension_4 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_4->Initialize());
  EXPECT_FALSE(invalid_extension_4->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_5/invalid_extension_5.dll"));
  TestExtension* invalid_extension_5 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_5->Initialize());
  EXPECT_FALSE(invalid_extension_5->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_6/invalid_extension_6.dll"));
  TestExtension* invalid_extension_6 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_FALSE(invalid_extension_6->Initialize());
  EXPECT_FALSE(invalid_extension_6->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_7/invalid_extension_7.dll"));
  TestExtension* invalid_extension_7 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_7->Initialize());
  EXPECT_FALSE(invalid_extension_7->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_8/invalid_extension_8.dll"));
  TestExtension* invalid_extension_8 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_8->Initialize());
  EXPECT_FALSE(invalid_extension_8->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_9/invalid_extension_9.dll"));
  TestExtension* invalid_extension_9 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_9->Initialize());
  EXPECT_FALSE(invalid_extension_9->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_10/invalid_extension_10.dll"));
  TestExtension* invalid_extension_10 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_10->Initialize());
  EXPECT_FALSE(invalid_extension_10->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_11/invalid_extension_11.dll"));
  TestExtension* invalid_extension_11 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_11->Initialize());
  EXPECT_FALSE(invalid_extension_11->CreateDotNetInstance()->isValid());

  test_path = GetDotNetExtensionTestPath(
    FILE_PATH_LITERAL("invalid_extension_12/invalid_extension_12.dll"));
  TestExtension* invalid_extension_12 = new TestExtension(test_path);
  EXPECT_TRUE(base::PathExists(test_path));
  EXPECT_TRUE(invalid_extension_12->Initialize());
  EXPECT_FALSE(invalid_extension_12->CreateDotNetInstance()->isValid());
}
