// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/base64.h"
#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_reader.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/strings/string_split.h"
#include "base/values.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome_launcher.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(ProcessExtensions, NoExtension) {
  Switches switches;
  std::vector<std::string> extensions;
  base::FilePath extension_dir;
  std::vector<std::string> bg_pages;
  Status status = internal::ProcessExtensions(extensions, extension_dir,
                                              false, &switches, &bg_pages);
  ASSERT_TRUE(status.IsOk());
  ASSERT_FALSE(switches.HasSwitch("load-extension"));
  ASSERT_EQ(0u, bg_pages.size());
}

bool AddExtensionForInstall(const std::string& relative_path,
                            std::vector<std::string>* extensions) {
  base::FilePath source_root;
  PathService::Get(base::DIR_SOURCE_ROOT, &source_root);
  base::FilePath crx_file_path = source_root.AppendASCII(
      "chrome/test/data/chromedriver/" + relative_path);
  std::string crx_contents;
  if (!base::ReadFileToString(crx_file_path, &crx_contents))
    return false;

  std::string crx_encoded;
  if (!base::Base64Encode(crx_contents, &crx_encoded))
    return false;
  extensions->push_back(crx_encoded);
  return true;
}

TEST(ProcessExtensions, SingleExtensionWithBgPage) {
  std::vector<std::string> extensions;
  ASSERT_TRUE(AddExtensionForInstall("ext_slow_loader.crx", &extensions));

  base::ScopedTempDir extension_dir;
  ASSERT_TRUE(extension_dir.CreateUniqueTempDir());

  Switches switches;
  std::vector<std::string> bg_pages;
  Status status = internal::ProcessExtensions(extensions, extension_dir.path(),
                                              false, &switches, &bg_pages);
  ASSERT_TRUE(status.IsOk());
  ASSERT_TRUE(switches.HasSwitch("load-extension"));
  base::FilePath temp_ext_path(switches.GetSwitchValueNative("load-extension"));
  ASSERT_TRUE(base::PathExists(temp_ext_path));
  std::string manifest_txt;
  ASSERT_TRUE(base::ReadFileToString(
      temp_ext_path.AppendASCII("manifest.json"), &manifest_txt));
  scoped_ptr<base::Value> manifest(base::JSONReader::Read(manifest_txt));
  ASSERT_TRUE(manifest);
  base::DictionaryValue* manifest_dict = NULL;
  ASSERT_TRUE(manifest->GetAsDictionary(&manifest_dict));
  std::string key;
  ASSERT_TRUE(manifest_dict->GetString("key", &key));
  ASSERT_EQ(
      "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC8qhZthEHjTIA3IYMzi79s2KFepVziY0du"
      "JzHcqRUB/YHSGseIUqcYXGazJhDz/"
      "4FbRg8ef9fQazL1UbMMGBIf4za1kJ2os2MsRrNXzHslkbtcLVj2VfofhuHJmu+"
      "CnKJ77UWamJiNAaQSiclu4duwnEWrkx+g/8ChQfhZzC4jvQIDAQAB",
      key);
  ASSERT_EQ(1u, bg_pages.size());
  ASSERT_EQ(
      "chrome-extension://jijhlkpcmmeckhlgdipjhnchhoabdjae/"
      "_generated_background_page.html",
      bg_pages[0]);
}

TEST(ProcessExtensions, MultipleExtensionsNoBgPages) {
  std::vector<std::string> extensions;
  ASSERT_TRUE(AddExtensionForInstall("ext_test_1.crx", &extensions));
  ASSERT_TRUE(AddExtensionForInstall("ext_test_2.crx", &extensions));

  base::ScopedTempDir extension_dir;
  ASSERT_TRUE(extension_dir.CreateUniqueTempDir());

  Switches switches;
  std::vector<std::string> bg_pages;
  Status status = internal::ProcessExtensions(extensions, extension_dir.path(),
                                              false, &switches, &bg_pages);
  ASSERT_TRUE(status.IsOk());
  ASSERT_TRUE(switches.HasSwitch("load-extension"));
  CommandLine::StringType ext_paths = switches.GetSwitchValueNative(
      "load-extension");
  std::vector<CommandLine::StringType> ext_path_list;
  base::SplitString(ext_paths, FILE_PATH_LITERAL(','), &ext_path_list);
  ASSERT_EQ(2u, ext_path_list.size());
  ASSERT_TRUE(base::PathExists(base::FilePath(ext_path_list[0])));
  ASSERT_TRUE(base::PathExists(base::FilePath(ext_path_list[1])));
  ASSERT_EQ(0u, bg_pages.size());
}

TEST(ProcessExtensions, CommandLineExtensions) {
  std::vector<std::string> extensions;
  ASSERT_TRUE(AddExtensionForInstall("ext_test_1.crx", &extensions));
  base::ScopedTempDir extension_dir;
  ASSERT_TRUE(extension_dir.CreateUniqueTempDir());

  Switches switches;
  switches.SetSwitch("load-extension", "/a");
  std::vector<std::string> bg_pages;
  Status status = internal::ProcessExtensions(extensions, extension_dir.path(),
                                              false, &switches, &bg_pages);
  ASSERT_EQ(kOk, status.code());
  base::FilePath::StringType load = switches.GetSwitchValueNative(
      "load-extension");
  ASSERT_EQ(FILE_PATH_LITERAL("/a,"), load.substr(0, 3));
  ASSERT_TRUE(base::PathExists(base::FilePath(load.substr(3))));
}

namespace {

void AssertEQ(const base::DictionaryValue& dict, const std::string& key,
              const char* expected_value) {
  std::string value;
  ASSERT_TRUE(dict.GetString(key, &value));
  ASSERT_STREQ(value.c_str(), expected_value);
}

}  // namespace

TEST(PrepareUserDataDir, CustomPrefs) {
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());

  base::DictionaryValue prefs;
  prefs.SetString("myPrefsKey", "ok");
  prefs.SetStringWithoutPathExpansion("pref.sub", "1");
  base::DictionaryValue local_state;
  local_state.SetString("myLocalKey", "ok");
  local_state.SetStringWithoutPathExpansion("local.state.sub", "2");
  Status status = internal::PrepareUserDataDir(
      temp_dir.path(), &prefs, &local_state);
  ASSERT_EQ(kOk, status.code());

  base::FilePath prefs_file =
      temp_dir.path().AppendASCII("Default").AppendASCII("Preferences");
  std::string prefs_str;
  ASSERT_TRUE(base::ReadFileToString(prefs_file, &prefs_str));
  scoped_ptr<base::Value> prefs_value(base::JSONReader::Read(prefs_str));
  const base::DictionaryValue* prefs_dict = NULL;
  ASSERT_TRUE(prefs_value->GetAsDictionary(&prefs_dict));
  AssertEQ(*prefs_dict, "myPrefsKey", "ok");
  AssertEQ(*prefs_dict, "pref.sub", "1");

  base::FilePath local_state_file = temp_dir.path().AppendASCII("Local State");
  std::string local_state_str;
  ASSERT_TRUE(base::ReadFileToString(local_state_file, &local_state_str));
  scoped_ptr<base::Value> local_state_value(
      base::JSONReader::Read(local_state_str));
  const base::DictionaryValue* local_state_dict = NULL;
  ASSERT_TRUE(local_state_value->GetAsDictionary(&local_state_dict));
  AssertEQ(*local_state_dict, "myLocalKey", "ok");
  AssertEQ(*local_state_dict, "local.state.sub", "2");
}
