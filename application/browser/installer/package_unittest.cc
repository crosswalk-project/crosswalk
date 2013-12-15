// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/package.h"

#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {
namespace application {

// As of now only XPK unit tests are present
class PackageTest : public testing::Test {
 public:
  void SetupPackage(const std::string& package_name) {
    base::FilePath package_path;
    ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &package_path));
    package_path = package_path.AppendASCII("xwalk")
        .AppendASCII("application")
        .AppendASCII("test")
        .AppendASCII("unpacker")
        .AppendASCII(package_name);
    ASSERT_TRUE(base::PathExists(package_path)) << package_path.value();

    package_ = Package::Create(package_path);
  }

 protected:
  scoped_ptr<Package> package_;
  base::ScopedTempDir temp_dir_;
};

TEST_F(PackageTest, Good) {
  SetupPackage("good.xpk");
  EXPECT_FALSE(package_->Id().empty());
  base::FilePath path;
  EXPECT_TRUE(package_->Extract(&path));
  EXPECT_TRUE(base::DirectoryExists(path));
  EXPECT_TRUE(temp_dir_.Set(path));
}

TEST_F(PackageTest, BadMagicString) {
  SetupPackage("bad_magic.xpk");
  base::FilePath path;
  EXPECT_FALSE(package_->Extract(&path));
}

TEST_F(PackageTest, BadSignature) {
  SetupPackage("bad_signature.xpk");
  base::FilePath path;
  EXPECT_FALSE(package_->Extract(&path));
}

TEST_F(PackageTest, NoMagicHeader) {
  SetupPackage("no_magic_header.xpk");
  base::FilePath path;
  EXPECT_FALSE(package_->Extract(&path));
}

TEST_F(PackageTest, BadXPKPackageExtension) {
  SetupPackage("error.ext");
  base::FilePath path;
  EXPECT_TRUE(package_ == NULL);
}

TEST_F(PackageTest, BadUnzipFile) {
  SetupPackage("bad_zip.xpk");
  base::FilePath path;
  EXPECT_FALSE(package_->Extract(&path));
}

// .wgt unittest
TEST_F(PackageTest, GoodWgt) {
  SetupPackage("hangoman.wgt");
  base::FilePath path;
  EXPECT_TRUE(package_->Extract(&path));
  EXPECT_TRUE(base::DirectoryExists(path));
  EXPECT_TRUE(temp_dir_.Set(path));
}

}  // namespace application
}  // namespace xwalk
