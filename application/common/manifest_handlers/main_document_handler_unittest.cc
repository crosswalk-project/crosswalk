// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/main_document_handler.h"

#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

class MainDocumentHandlerTest : public testing::Test {
 public:
  virtual void SetUp() OVERRIDE {
    manifest.SetString(keys::kNameKey, "no name");
    manifest.SetString(keys::kVersionKey, "0");
  }

  scoped_refptr<ApplicationData> CreateApplication() {
    std::string error;
    scoped_refptr<ApplicationData> application = ApplicationData::Create(
        base::FilePath(), Manifest::INVALID_TYPE, manifest, "", &error);
    return application;
  }

  const MainDocumentInfo* GetMainDocInfo(const ApplicationData* application) {
    return ToMainDocumentInfo(application->GetManifestData(keys::kAppMainKey));
  }

  base::DictionaryValue manifest;
};

TEST_F(MainDocumentHandlerTest, MainSource) {
  manifest.SetString(keys::kAppMainSourceKey, "1.html");
  scoped_refptr<ApplicationData> application = CreateApplication();

  ASSERT_TRUE(application.get());
  EXPECT_EQ(GetMainDocInfo(application)->GetMainURL(),
            application->GetResourceURL("1.html"));
}

TEST_F(MainDocumentHandlerTest, MainScripts) {
  base::ListValue* scripts = new base::ListValue;
  scripts->AppendString("1.js");
  scripts->AppendString("2.js");
  manifest.Set(keys::kAppMainScriptsKey, scripts);
  scoped_refptr<ApplicationData> application = CreateApplication();

  ASSERT_TRUE(application.get());
  EXPECT_EQ(GetMainDocInfo(application)->GetMainURL(),
            application->GetResourceURL(kGeneratedMainDocumentFilename));
  EXPECT_EQ(GetMainDocInfo(application)->GetMainScripts().size(), 2);

  scripts->AppendInteger(1);
  application = CreateApplication();
  EXPECT_FALSE(application.get());
}

// When both main.source and main.scripts are defined in manifest, the source
// will be used with higher preference.
TEST_F(MainDocumentHandlerTest, SourceAndScripts) {
  manifest.SetString(keys::kAppMainSourceKey, "1.html");
  base::ListValue* scripts = new base::ListValue;
  scripts->AppendString("1.js");
  manifest.Set(keys::kAppMainScriptsKey, scripts);
  scoped_refptr<ApplicationData> application = CreateApplication();

  ASSERT_TRUE(application.get());
  EXPECT_EQ(GetMainDocInfo(application)->GetMainURL(),
            application->GetResourceURL("1.html"));
  EXPECT_FALSE(GetMainDocInfo(application)->IsPersistent());
}

TEST_F(MainDocumentHandlerTest, MainPersistent) {
  manifest.SetString(keys::kAppMainSourceKey, "1.html");
  manifest.SetBoolean(keys::kAppMainPersistentKey, true);
  scoped_refptr<ApplicationData> application = CreateApplication();

  ASSERT_TRUE(application.get());
  EXPECT_TRUE(GetMainDocInfo(application)->IsPersistent());
}

}  // namespace application
}  // namespace xwalk
