// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/xpk_extractor.h"

#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {
namespace application {

class XPKExtractorTest : public testing::Test {
 public:
  virtual ~XPKExtractorTest() {
    temp_dir_.Delete();
  }

  void SetupXPKExtractor(const std::string& xpk_name) {
    base::FilePath xpk_path;
    ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &xpk_path));
    xpk_path = xpk_path.AppendASCII("xwalk")
        .AppendASCII("application")
        .AppendASCII("test")
        .AppendASCII("unpacker")
        .AppendASCII(xpk_name);
    ASSERT_TRUE(file_util::PathExists(xpk_path)) << xpk_path.value();

    extractor_ = XPKExtractor::Create(xpk_path);
  }

 protected:
  base::ScopedTempDir temp_dir_;
  scoped_refptr<XPKExtractor> extractor_;
};

TEST_F(XPKExtractorTest, Good) {
  SetupXPKExtractor("good.xpk");
  EXPECT_FALSE(extractor_->GetPackageID().empty());
  base::FilePath path;
  EXPECT_TRUE(extractor_->Extract(&path));
  EXPECT_TRUE(file_util::DirectoryExists(path));
  EXPECT_TRUE(temp_dir_.Set(path));
}

TEST_F(XPKExtractorTest, BadMagicString) {
  SetupXPKExtractor("bad_magic.xpk");
  base::FilePath path;
  EXPECT_FALSE(extractor_->Extract(&path));
}

TEST_F(XPKExtractorTest, BadSignature) {
  SetupXPKExtractor("bad_signature.xpk");
  base::FilePath path;
  EXPECT_FALSE(extractor_->Extract(&path));
}

TEST_F(XPKExtractorTest, NoMagicHeader) {
  SetupXPKExtractor("no_magic_header.xpk");
  base::FilePath path;
  EXPECT_FALSE(extractor_->Extract(&path));
}

TEST_F(XPKExtractorTest, BadXPKPackageExtension) {
  SetupXPKExtractor("error.ext");
  base::FilePath path;
  EXPECT_TRUE(extractor_ == NULL);
}

TEST_F(XPKExtractorTest, BadUnzipFile) {
  SetupXPKExtractor("bad_zip.xpk");
  base::FilePath path;
  EXPECT_FALSE(extractor_->Extract(&path));
}

}  // namespace application
}  // namespace xwalk
