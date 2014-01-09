// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/sysapps_manager.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/stl_util.h"
#include "media/base/media.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"
#include "xwalk/sysapps/device_capabilities_new/av_codecs_provider.h"
#include "xwalk/sysapps/device_capabilities_new/cpu_info_provider.h"

using xwalk::sysapps::AVCodecsProvider;
using xwalk::sysapps::CPUInfoProvider;
using xwalk::sysapps::SysAppsManager;
using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionVector;

namespace {

class XWalkSysAppsManagerTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    media::InitializeMediaLibraryForTesting();

    // We need to make sure the resource bundle is up because
    // the extensions we instantiate on this test depend on it.
    base::FilePath pak_dir;
    PathService::Get(base::DIR_MODULE, &pak_dir);

    ASSERT_FALSE(pak_dir.empty());

    base::FilePath pak_file = pak_dir.Append(FILE_PATH_LITERAL("xwalk.pak"));
    ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
  }
};

class DummyExtension : public XWalkExtension {
 public:
  DummyExtension() {}

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return NULL;
  }
};

int CountExtensions(SysAppsManager* manager) {
  XWalkExtensionVector extensions;
  STLElementDeleter<XWalkExtensionVector> deleter(&extensions);
  manager->CreateExtensionsForExtensionThread(&extensions);
  manager->CreateExtensionsForUIThread(&extensions);
  return extensions.size();
}

}  // namespace

TEST_F(XWalkSysAppsManagerTest, DisableDeviceCapabilities) {
  SysAppsManager manager;
  int count_before_disable = CountExtensions(&manager);
  manager.DisableDeviceCapabilities();
  int count_after_disable = CountExtensions(&manager);
  EXPECT_EQ(count_before_disable, count_after_disable + 1);
}

TEST_F(XWalkSysAppsManagerTest, DisableRawSockets) {
  SysAppsManager manager;
  int count_before_disable = CountExtensions(&manager);
  manager.DisableRawSockets();
  int count_after_disable = CountExtensions(&manager);
  EXPECT_EQ(count_before_disable, count_after_disable + 1);
}

TEST_F(XWalkSysAppsManagerTest, DoesNotReplaceExtensions) {
  XWalkExtension* extension_ptr(new DummyExtension);

  XWalkExtensionVector extensions;
  extensions.push_back(extension_ptr);

  SysAppsManager manager;
  manager.CreateExtensionsForExtensionThread(&extensions);
  EXPECT_GE(extensions.size(), 1u);

  manager.CreateExtensionsForUIThread(&extensions);
  EXPECT_GE(extensions.size(), 1u);

  EXPECT_EQ(extensions[0], extension_ptr);

  STLDeleteElements(&extensions);
}

TEST_F(XWalkSysAppsManagerTest, GetAVCodecsProvider) {
  SysAppsManager manager;

  AVCodecsProvider* provider(manager.GetAVCodecsProvider());
  EXPECT_TRUE(provider != NULL);

  // AVCodecsProvider is shared among different extensions
  // instances. GetAVCodecsProvider() should always return
  // the same provider.
  EXPECT_EQ(provider, manager.GetAVCodecsProvider());
}

TEST_F(XWalkSysAppsManagerTest, GetCPUProvider) {
  SysAppsManager manager;

  CPUInfoProvider* provider(manager.GetCPUInfoProvider());
  EXPECT_TRUE(provider != NULL);

  // CPUInfoProvider is shared among different extensions
  // instances. GetCPUInfoProvider() should always return
  // the same provider.
  EXPECT_EQ(provider, manager.GetCPUInfoProvider());
}
