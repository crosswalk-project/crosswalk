// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/install_warning.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace errors = application_manifest_errors;
namespace keys = application_manifest_keys;

namespace xwalk_application {

class ManifestTest : public testing::Test {
 public:
  ManifestTest() : default_value_("test") {}

 protected:
  void AssertType(Manifest* manifest, Manifest::Type type) {
    EXPECT_EQ(type, manifest->type());
    EXPECT_EQ(type == Manifest::TYPE_PLATFORM_APP,
              manifest->is_platform_app());
    EXPECT_EQ(type == Manifest::TYPE_HOSTED_APP, manifest->is_hosted_app());
  }

  // Helper function that replaces the Manifest held by |manifest| with a copy
  // with its |key| changed to |value|. If |value| is NULL, then |key| will
  // instead be deleted.
  void MutateManifest(scoped_ptr<Manifest>* manifest,
                      const std::string& key,
                      base::Value* value) {
    scoped_ptr<base::DictionaryValue> manifest_value(
        manifest->get()->value()->DeepCopy());
    if (value)
      manifest_value->Set(key, value);
    else
      manifest_value->Remove(key, NULL);
    manifest->reset(new Manifest(Manifest::UNPACKED, manifest_value.Pass()));
  }

  std::string default_value_;
};

// Verifies that application can access the correct keys.
TEST_F(ManifestTest, Application) {
  scoped_ptr<base::DictionaryValue> manifest_value(new base::DictionaryValue());
  manifest_value->SetString(keys::kName, "extension");
  manifest_value->SetString(keys::kVersion, "1");
  manifest_value->SetString("unknown_key", "foo");

  scoped_ptr<Manifest> manifest(
      new Manifest(Manifest::UNPACKED, manifest_value.Pass()));
  std::string error;
  std::vector<InstallWarning> warnings;
  EXPECT_TRUE(manifest->ValidateManifest(&error, &warnings));
  EXPECT_TRUE(error.empty());
  // TODO(xiang): warnings will not be empty after enable manifest features
  ASSERT_EQ(0u, warnings.size());
  // AssertType(manifest.get(), Manifest::TYPE_HOSTED_AP);

  // The unknown key 'unknown_key' should be accesible.
  std::string value;
  EXPECT_TRUE(manifest->GetString("unknown_key", &value));
  EXPECT_EQ("foo", value);

  // Test DeepCopy and Equals.
  scoped_ptr<Manifest> manifest2(manifest->DeepCopy());
  EXPECT_TRUE(manifest->Equals(manifest2.get()));
  EXPECT_TRUE(manifest2->Equals(manifest.get()));
  MutateManifest(
      &manifest, "foo", new base::StringValue("blah"));
  EXPECT_FALSE(manifest->Equals(manifest2.get()));
};

// Verifies that key restriction based on type works.
TEST_F(ManifestTest, ApplicationTypes) {
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue());
  value->SetString(keys::kName, "extension");
  value->SetString(keys::kVersion, "1");

  scoped_ptr<Manifest> manifest(
      new Manifest(Manifest::UNPACKED, value.Pass()));
  std::string error;
  std::vector<InstallWarning> warnings;
  EXPECT_TRUE(manifest->ValidateManifest(&error, &warnings));
  EXPECT_TRUE(error.empty());
  EXPECT_TRUE(warnings.empty());

  // Platform app.
  MutateManifest(
      &manifest, keys::kPlatformAppBackground, new base::DictionaryValue());
  AssertType(manifest.get(), Manifest::TYPE_PLATFORM_APP);
  MutateManifest(
      &manifest, keys::kPlatformAppBackground, NULL);

  // Hosted app.
  MutateManifest(
      &manifest, keys::kWebURLs, new base::ListValue());
  AssertType(manifest.get(), Manifest::TYPE_HOSTED_APP);
  MutateManifest(
      &manifest, keys::kWebURLs, NULL);
  MutateManifest(
      &manifest, keys::kLaunchWebURL, new base::StringValue("foo"));
  AssertType(manifest.get(), Manifest::TYPE_HOSTED_APP);
  MutateManifest(
      &manifest, keys::kLaunchWebURL, NULL);
};

}  // namespace xwalk_application


