// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/csp_handler.h"

#include "xwalk/application/common/application_manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

class CSPHandlerTest: public testing::Test {
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

  const CSPInfo* GetCSPInfo(
      scoped_refptr<ApplicationData> application) {
    const CSPInfo* info = static_cast<CSPInfo*>(
        application->GetManifestData(keys::kCSPKey));
    DCHECK(info);
    return info;
  }

  base::DictionaryValue manifest;
};

// FIXME: the default CSP policy settings in CSP manifest handler
// are temporally removed, since they had affected some tests and legacy apps.
TEST_F(CSPHandlerTest, DISABLED_NoCSP) {
  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  EXPECT_EQ(GetCSPInfo(application)->GetDirectives().size(), 2);
}

TEST_F(CSPHandlerTest, EmptyCSP) {
  manifest.SetString(keys::kCSPKey, "");
  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  EXPECT_EQ(GetCSPInfo(application)->GetDirectives().size(), 0);
}

TEST_F(CSPHandlerTest, CSP) {
  manifest.SetString(keys::kCSPKey, "default-src    'self'   ");
  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  const std::map<std::string, std::vector<std::string> >& policies =
      GetCSPInfo(application)->GetDirectives();
  EXPECT_EQ(policies.size(), 1);
  std::map<std::string, std::vector<std::string> >::const_iterator it =
      policies.find("default-src");
  ASSERT_NE(it, policies.end());
  EXPECT_EQ(it->second.size(), 1);
  EXPECT_STREQ((it->second)[0].c_str(), "'self'");
}

}  // namespace application
}  // namespace xwalk
