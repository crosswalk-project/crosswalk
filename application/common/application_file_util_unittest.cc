// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_file_util.h"

#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_string_value_serializer.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/constants.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/l10n/l10n_util.h"

using xwalk::application::Application;
using xwalk::application::Manifest;

namespace keys = xwalk::application_manifest_keys;

namespace xwalk {
namespace application {

class ApplicationFileUtilTest : public testing::Test {
};

TEST_F(ApplicationFileUtilTest, LoadApplicationWithValidPath) {
  base::FilePath install_dir;
  ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &install_dir));
  install_dir = install_dir.AppendASCII("xwalk")
      .AppendASCII("application")
      .AppendASCII("test")
      .AppendASCII("data")
      .AppendASCII("good")
      .AppendASCII("Applications")
      .AppendASCII("aaa");

  std::string error;
  scoped_refptr<Application> application(LoadApplication(
          install_dir, Manifest::COMMAND_LINE, &error));
  ASSERT_TRUE(application != NULL);
  EXPECT_EQ("The first application that I made.", application->Description());
}

TEST_F(ApplicationFileUtilTest,
       LoadApplicationGivesHelpfullErrorOnMissingManifest) {
  base::FilePath install_dir;
  ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &install_dir));
  install_dir = install_dir.AppendASCII("xwalk")
      .AppendASCII("application")
      .AppendASCII("test")
      .AppendASCII("data")
      .AppendASCII("bad")
      .AppendASCII("Applications")
      .AppendASCII("aaa");

  std::string error;
  scoped_refptr<Application> application(LoadApplication(
          install_dir, Manifest::COMMAND_LINE, &error));
  ASSERT_TRUE(application == NULL);
  ASSERT_FALSE(error.empty());
  ASSERT_STREQ("Manifest file is missing or unreadable.", error.c_str());
}

TEST_F(ApplicationFileUtilTest,
       LoadApplicationGivesHelpfullErrorOnBadManifest) {
  base::FilePath install_dir;
  ASSERT_TRUE(PathService::Get(base::DIR_SOURCE_ROOT, &install_dir));
  install_dir = install_dir.AppendASCII("xwalk")
      .AppendASCII("application")
      .AppendASCII("test")
      .AppendASCII("data")
      .AppendASCII("bad")
      .AppendASCII("Applications")
      .AppendASCII("bbb");

  std::string error;
  scoped_refptr<Application> application(LoadApplication(
          install_dir, Manifest::COMMAND_LINE, &error));
  ASSERT_TRUE(application == NULL);
  ASSERT_FALSE(error.empty());
  ASSERT_STREQ("Manifest is not valid JSON."
               "  Line: 2, column: 16, Syntax error.",
               error.c_str());
}

static scoped_refptr<Application> LoadApplicationManifest(
    base::DictionaryValue* manifest,
    const base::FilePath& manifest_dir,
    Manifest::SourceType location,
    int extra_flags,
    std::string* error) {
  scoped_refptr<Application> application = Application::Create(
      manifest_dir, location, *manifest, std::string(), error);
  return application;
}

static scoped_refptr<Application> LoadApplicationManifest(
    const std::string& manifest_value,
    const base::FilePath& manifest_dir,
    Manifest::SourceType location,
    int extra_flags,
    std::string* error) {
  JSONStringValueSerializer serializer(manifest_value);
  scoped_ptr<base::Value> result(serializer.Deserialize(NULL, error));
  if (!result.get())
    return NULL;
  CHECK_EQ(base::Value::TYPE_DICTIONARY, result->GetType());
  return LoadApplicationManifest(
      static_cast<base::DictionaryValue*>(result.get()),
      manifest_dir,
      location,
      extra_flags,
      error);
}

TEST_F(ApplicationFileUtilTest, ValidateThemeUTF8) {
  base::ScopedTempDir temp;
  ASSERT_TRUE(temp.CreateUniqueTempDir());

  // "aeo" with accents. Use http://0xcc.net/jsescape/ to decode them.
  std::string non_ascii_file = "\xC3\xA0\xC3\xA8\xC3\xB2.png";
  base::FilePath non_ascii_path =
      temp.path().Append(base::FilePath::FromUTF8Unsafe(non_ascii_file));
  file_util::WriteFile(non_ascii_path, "", 0);

  std::string kManifest =
      base::StringPrintf(
          "{ \"name\": \"Test\", \"version\": \"1.0\", "
          "  \"theme\": { \"images\": { \"theme_frame\": \"%s\" } }"
          "}", non_ascii_file.c_str());
  std::string error;
  scoped_refptr<Application> application = LoadApplicationManifest(
      kManifest, temp.path(), Manifest::COMMAND_LINE, 0, &error);
  ASSERT_TRUE(application.get()) << error;
}

}  // namespace application
}  // namespace xwalk
