// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace errors = xwalk::application_manifest_errors;
namespace keys = xwalk::application_manifest_keys;

namespace xwalk {
namespace application {

class ManifestTest : public testing::Test {
 public:
  ManifestTest() : default_value_("test") {}

 protected:
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
    manifest->reset(new Manifest(manifest_value.Pass()));
  }

  std::string default_value_;
};

// Verifies that application can access the correct keys.
TEST_F(ManifestTest, ApplicationData) {
  scoped_ptr<base::DictionaryValue> manifest_value(new base::DictionaryValue());
  manifest_value->SetString(keys::kNameKey, "extension");
  manifest_value->SetString(keys::kXWalkVersionKey, "1");
  manifest_value->SetString("unknown_key", "foo");

  scoped_ptr<Manifest> manifest(
      new Manifest(manifest_value.Pass()));
  std::string error;
  EXPECT_TRUE(manifest->ValidateManifest(&error));
  EXPECT_TRUE(error.empty());

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
}

// Verifies that key restriction based on type works.
TEST_F(ManifestTest, ApplicationTypes) {
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue());
  value->SetString(keys::kNameKey, "extension");
  value->SetString(keys::kXWalkVersionKey, "1");

  scoped_ptr<Manifest> manifest(
      new Manifest(value.Pass()));
  std::string error;
  EXPECT_TRUE(manifest->ValidateManifest(&error));
  EXPECT_TRUE(error.empty());
}

}  // namespace application
}  // namespace xwalk


