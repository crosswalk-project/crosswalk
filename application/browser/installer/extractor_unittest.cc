// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/extractor.h"

#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {
namespace application {

class ExtractorTest : public testing::Test {
 public:
  void SetupXPKExtractor(const std::string& xpk_name) {
    base::FilePath xpk_path;
    ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &xpk_path));
    xpk_path = xpk_path.AppendASCII("xwalk")
        .AppendASCII("application")
        .AppendASCII("test")
        .AppendASCII("unpacker")
        .AppendASCII(xpk_name);
    ASSERT_TRUE(base::PathExists(xpk_path)) << xpk_path.value();

    extractor_ = Extractor::Create(xpk_path);
  }

 protected:
  base::ScopedTempDir temp_dir_;
  scoped_ptr<Extractor> extractor_;
};

TEST_F(ExtractorTest, Good) {
  SetupExtractor("good.xpk");
  EXPECT_FALSE(extractor_->GetPackageID().empty());
  base::FilePath path;
  EXPECT_TRUE(extractor_->Extract(&path));
  EXPECT_TRUE(base::DirectoryExists(path));
  EXPECT_TRUE(temp_dir_.Set(path));
}

TEST_F(ExtractorTest, BadMagicString) {
  SetupExtractor("bad_magic.xpk");
  base::FilePath path;
  EXPECT_FALSE(extractor_->Extract(&path));
}

TEST_F(ExtractorTest, BadSignature) {
  SetupExtractor("bad_signature.xpk");
  base::FilePath path;
  EXPECT_FALSE(extractor_->Extract(&path));
}

TEST_F(ExtractorTest, NoMagicHeader) {
  SetupExtractor("no_magic_header.xpk");
  base::FilePath path;
  EXPECT_FALSE(extractor_->Extract(&path));
}

TEST_F(ExtractorTest, BadXPKPackageExtension) {
  SetupExtractor("error.ext");
  base::FilePath path;
  EXPECT_TRUE(extractor_ == NULL);
}

TEST_F(ExtractorTest, BadUnzipFile) {
  SetupExtractor("bad_zip.xpk");
  base::FilePath path;
  EXPECT_FALSE(extractor_->Extract(&path));
}

}  // namespace application
}  // namespace xwalk
