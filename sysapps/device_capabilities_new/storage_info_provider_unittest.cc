// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/storage_info_provider.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/sysapps/common/sysapps_manager.h"
#include "xwalk/sysapps/device_capabilities_new/device_capabilities.h"

using xwalk::jsapi::device_capabilities::StorageUnit;
using xwalk::jsapi::device_capabilities::SystemStorage;
using xwalk::sysapps::StorageInfoProvider;
using xwalk::sysapps::SysAppsManager;

namespace {

class TestObserver : public StorageInfoProvider::Observer {
 public:
  TestObserver() {}

 private:
  virtual void OnStorageAttached(const StorageUnit& storage) OVERRIDE {}
  virtual void OnStorageDetached(const StorageUnit& storage) OVERRIDE {}
};

void TestClosure() {
  StorageInfoProvider* provider(SysAppsManager::GetStorageInfoProvider());
  EXPECT_TRUE(provider != NULL);
  EXPECT_TRUE(provider->IsInitialized());

  scoped_ptr<SystemStorage> info(provider->storage_info());
  EXPECT_TRUE(info != NULL);

  std::vector<linked_ptr<StorageUnit> > storages = info->storages;

  // We should have at least one storage, otherwise where is the binary
  // of this unit test being stored?
  unsigned storage_count = storages.size();
  EXPECT_GT(storage_count, 0u);

  // The only information we can verify is the fact that the storage
  // has some capacity and has an ID. The rest, like the name, can be empty.
  for (size_t i = 0; i < storage_count; ++i) {
    EXPECT_FALSE(storages[i]->id.empty());
    EXPECT_GE(storages[i]->capacity, 0);
  }

  // We can just test if adding and removing the observers works, but
  // without a fake backend, we cannot verify if the observers are getting
  // called.
  TestObserver test_observer;
  provider->AddObserver(&test_observer);
  provider->RemoveObserver(&test_observer);

  base::MessageLoop::current()->Quit();
}

}  // namespace

TEST(XWalkSysAppsDeviceCapabilitiesTest, StorageInfoProvider) {
  base::MessageLoop message_loop;

  StorageInfoProvider* provider(SysAppsManager::GetStorageInfoProvider());
  EXPECT_TRUE(provider != NULL);

  base::Closure closure = base::Bind(&TestClosure);

  if (!provider->IsInitialized())
    provider->AddOnInitCallback(closure);
  else
    message_loop.PostTask(FROM_HERE, closure);

  message_loop.Run();
}
