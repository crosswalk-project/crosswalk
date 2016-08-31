// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/csp_handler.h"
#include "xwalk/application/common/manifest_handlers/unittest_util.h"

namespace xwalk {

namespace keys = application_manifest_keys;
namespace widget_keys = application_widget_keys;

namespace application {

namespace {

const CSPInfo* GetCSPInfo(
    scoped_refptr<ApplicationData> application) {
  const CSPInfo* info = static_cast<CSPInfo*>(
      application->GetManifestData(GetCSPKey(application->manifest_type())));
  return info;
}

}  // namespace

class CSPHandlerTest: public testing::Test {
};

// FIXME: the default CSP policy settings in CSP manifest handler
// are temporally removed, since they had affected some tests and legacy apps.
TEST_F(CSPHandlerTest, DISABLED_NoCSP) {
  std::unique_ptr<base::DictionaryValue> manifest = CreateDefaultManifestConfig();
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_MANIFEST, *manifest);
  EXPECT_TRUE(application.get());
  EXPECT_EQ(GetCSPInfo(application)->GetDirectives().size(), 2u);
}

TEST_F(CSPHandlerTest, EmptyCSP) {
  std::unique_ptr<base::DictionaryValue> manifest = CreateDefaultManifestConfig();
  manifest->SetString(keys::kCSPKey, "");
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_MANIFEST, *manifest);
  EXPECT_TRUE(application.get());
  EXPECT_EQ(GetCSPInfo(application)->GetDirectives().size(), 0u);
}

TEST_F(CSPHandlerTest, CSP) {
  std::unique_ptr<base::DictionaryValue> manifest = CreateDefaultManifestConfig();
  manifest->SetString(keys::kCSPKey, "default-src    'self'   ");
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_MANIFEST, *manifest);
  EXPECT_TRUE(application.get());
  const std::map<std::string, std::vector<std::string> >& policies =
      GetCSPInfo(application)->GetDirectives();
  EXPECT_EQ(policies.size(), 1u);
  std::map<std::string, std::vector<std::string> >::const_iterator it =
      policies.find("default-src");
  ASSERT_NE(it, policies.end());
  EXPECT_EQ(it->second.size(), 1u);
  EXPECT_STREQ((it->second)[0].c_str(), "'self'");
}

}  // namespace application
}  // namespace xwalk
