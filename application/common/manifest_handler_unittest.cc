// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/stl_util.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/manifest_handler.h"
#include "xwalk/application/common/install_warning.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {
namespace application {

namespace {

std::vector<std::string> SingleKey(const std::string& key) {
  return std::vector<std::string>(1, key);
}

}  // namespace

class ScopedTestingManifestHandlerRegistry {
 public:
  ScopedTestingManifestHandlerRegistry(
      const std::vector<ManifestHandler*>& handlers)
      : registry_(
          new ManifestHandlerRegistry(handlers)),
        prev_registry_(ManifestHandlerRegistry::GetInstance()) {
    ManifestHandlerRegistry::SetInstanceForTesting(registry_.get());
  }

  ~ScopedTestingManifestHandlerRegistry() {
    ManifestHandlerRegistry::SetInstanceForTesting(prev_registry_);
  }

  scoped_ptr<ManifestHandlerRegistry> registry_;
  ManifestHandlerRegistry* prev_registry_;
};

class ManifestHandlerTest : public testing::Test {
 public:
  class ParsingWatcher {
   public:
    // Called when a manifest handler parses.
    void Record(const std::string& name) {
      parsed_names_.push_back(name);
    }

    const std::vector<std::string>& parsed_names() {
      return parsed_names_;
    }

    // Returns true if |name_before| was parsed before |name_after|.
    bool ParsedBefore(const std::string& name_before,
                      const std::string& name_after) {
      size_t prev_iterator = parsed_names_.size();
      size_t next_iterator = 0;
      for (size_t i = 0; i < parsed_names_.size(); ++i) {
        if (parsed_names_[i] == name_before)
          prev_iterator = i;
        if (parsed_names_[i] == name_after)
          next_iterator = i;
      }

      if (prev_iterator < next_iterator)
        return true;

      return false;
    }

   private:
    // The order of manifest handlers that we watched parsing.
    std::vector<std::string> parsed_names_;
  };

  class TestManifestHandler : public ManifestHandler {
   public:
    TestManifestHandler(const std::string& name,
                        const std::vector<std::string>& keys,
                        const std::vector<std::string>& prereqs,
                        ParsingWatcher* watcher)
        : name_(name), keys_(keys), prereqs_(prereqs), watcher_(watcher) {
    }

    virtual ~TestManifestHandler() {}

    virtual bool Parse(
        scoped_refptr<Application> application, string16* error) OVERRIDE {
      watcher_->Record(name_);
      return true;
    }

    virtual std::vector<std::string> PrerequisiteKeys() const OVERRIDE {
      return prereqs_;
    }

    virtual std::vector<std::string> Keys() const OVERRIDE {
      return keys_;
    }

   protected:
    std::string name_;
    std::vector<std::string> keys_;
    std::vector<std::string> prereqs_;
    ParsingWatcher* watcher_;
  };

  class FailingTestManifestHandler : public TestManifestHandler {
   public:
    FailingTestManifestHandler(const std::string& name,
                               const std::vector<std::string>& keys,
                               const std::vector<std::string>& prereqs,
                               ParsingWatcher* watcher)
        : TestManifestHandler(name, keys, prereqs, watcher) {
    }
    virtual bool Parse(
        scoped_refptr<Application> application, string16* error) OVERRIDE {
      *error = ASCIIToUTF16(name_);
      return false;
    }
  };

  class AlwaysParseTestManifestHandler : public TestManifestHandler {
   public:
    AlwaysParseTestManifestHandler(const std::string& name,
                                   const std::vector<std::string>& keys,
                                   const std::vector<std::string>& prereqs,
                                   ParsingWatcher* watcher)
        : TestManifestHandler(name, keys, prereqs, watcher) {
    }

    virtual bool AlwaysParseForType(Manifest::Type type) const OVERRIDE {
      return true;
    }
  };

  class TestManifestValidator : public ManifestHandler {
   public:
    TestManifestValidator(bool return_value,
                          bool always_validate,
                          std::vector<std::string> keys)
        : return_value_(return_value),
          always_validate_(always_validate),
          keys_(keys) {
    }

    virtual bool Parse(
        scoped_refptr<Application> application, string16* error) OVERRIDE {
      return true;
    }

    virtual bool Validate(
        scoped_refptr<const Application> application,
        std::string* error,
        std::vector<InstallWarning>* warnings) const OVERRIDE {
      return return_value_;
    }

    virtual bool AlwaysValidateForType(Manifest::Type type) const OVERRIDE {
      return always_validate_;
    }

    virtual std::vector<std::string> Keys() const OVERRIDE {
      return keys_;
    }

 protected:
    bool return_value_;
    bool always_validate_;
    std::vector<std::string> keys_;
  };
};

TEST_F(ManifestHandlerTest, DependentHandlers) {
  std::vector<ManifestHandler*> handlers;
  ParsingWatcher watcher;
  std::vector<std::string> prereqs;
  handlers.push_back(
      new TestManifestHandler("A", SingleKey("a"), prereqs, &watcher));
  handlers.push_back(
      new TestManifestHandler("B", SingleKey("b"), prereqs, &watcher));
  handlers.push_back(
      new TestManifestHandler("J", SingleKey("j"), prereqs, &watcher));
  handlers.push_back(
      new AlwaysParseTestManifestHandler(
          "K", SingleKey("k"), prereqs, &watcher));
  prereqs.push_back("c.d");
  std::vector<std::string> keys;
  keys.push_back("c.e");
  keys.push_back("c.z");
  handlers.push_back(
      new TestManifestHandler("C.EZ", keys, prereqs, &watcher));
  prereqs.clear();
  prereqs.push_back("b");
  prereqs.push_back("k");
  handlers.push_back(
      new TestManifestHandler("C.D", SingleKey("c.d"), prereqs, &watcher));
  ScopedTestingManifestHandlerRegistry registry(handlers);

  base::DictionaryValue manifest;
  manifest.SetString("name", "no name");
  manifest.SetString("version", "0");
  manifest.SetInteger("manifest_version", 2);
  manifest.SetInteger("a", 1);
  manifest.SetInteger("b", 2);
  manifest.SetInteger("c.d", 3);
  manifest.SetInteger("c.e", 4);
  manifest.SetInteger("c.f", 5);
  manifest.SetInteger("g", 6);
  std::string error;
  scoped_refptr<Application> application = Application::Create(
      base::FilePath(),
      Manifest::INVALID_TYPE,
      manifest,
      "",
      &error);
  EXPECT_TRUE(application.get());
  // A, B, C.EZ, C.D, K
  EXPECT_EQ(5u, watcher.parsed_names().size());
  EXPECT_TRUE(watcher.ParsedBefore("B", "C.D"));
  EXPECT_TRUE(watcher.ParsedBefore("K", "C.D"));
  EXPECT_TRUE(watcher.ParsedBefore("C.D", "C.EZ"));
}

TEST_F(ManifestHandlerTest, FailingHandlers) {
  scoped_ptr<ScopedTestingManifestHandlerRegistry> registry(
      new ScopedTestingManifestHandlerRegistry(
          std::vector<ManifestHandler*>()));
  // Can't use ApplicationBuilder, because this application will fail to
  // be parsed.
  base::DictionaryValue manifest_a;
  manifest_a.SetString("name", "no name");
  manifest_a.SetString("version", "0");
  manifest_a.SetInteger("manifest_version", 2);
  manifest_a.SetInteger("a", 1);

  // Succeeds when "a" is not recognized.
  std::string error;
  scoped_refptr<Application> application = Application::Create(
      base::FilePath(),
      Manifest::INVALID_TYPE,
      manifest_a,
      "",
      &error);
  EXPECT_TRUE(application.get());

  // Register a handler for "a" that fails.
  std::vector<ManifestHandler*> handlers;
  ParsingWatcher watcher;
  handlers.push_back(
      new FailingTestManifestHandler(
          "A", SingleKey("a"), std::vector<std::string>(), &watcher));
  registry.reset();
  registry.reset(new ScopedTestingManifestHandlerRegistry(handlers));

  application = Application::Create(
      base::FilePath(),
      Manifest::INVALID_TYPE,
      manifest_a,
      "",
      &error);
  EXPECT_FALSE(application.get());
  EXPECT_EQ("A", error);
}

TEST_F(ManifestHandlerTest, Validate) {
  scoped_ptr<ScopedTestingManifestHandlerRegistry> registry(
      new ScopedTestingManifestHandlerRegistry(
          std::vector<ManifestHandler*>()));
  base::DictionaryValue manifest;
  manifest.SetString("name", "no name");
  manifest.SetString("version", "0");
  manifest.SetInteger("manifest_version", 2);
  manifest.SetInteger("a", 1);
  manifest.SetInteger("b", 2);
  std::string error;
  scoped_refptr<Application> application = Application::Create(
      base::FilePath(),
      Manifest::COMMAND_LINE,
      manifest,
      "",
      &error);
  EXPECT_TRUE(application.get());

  std::vector<ManifestHandler*> handlers;
  std::vector<InstallWarning> warnings;
  // Always validates and fails.
  handlers.push_back(
      new TestManifestValidator(false, true, SingleKey("c")));
  registry.reset();
  registry.reset(new ScopedTestingManifestHandlerRegistry(handlers));
  EXPECT_FALSE(
      registry->registry_->ValidateAppManifest(application, &error, &warnings));

  handlers.push_back(
      new TestManifestValidator(false, false, SingleKey("c")));
  registry.reset();
  registry.reset(new ScopedTestingManifestHandlerRegistry(handlers));
  EXPECT_TRUE(
      registry->registry_->ValidateAppManifest(application, &error, &warnings));

  // Validates "a" and fails.
  handlers.push_back
      (new TestManifestValidator(false, true, SingleKey("a")));
  registry.reset();
  registry.reset(new ScopedTestingManifestHandlerRegistry(handlers));
  EXPECT_FALSE(
      registry->registry_->ValidateAppManifest(application, &error, &warnings));
}

}  // namespace application
}  // namespace xwalk
