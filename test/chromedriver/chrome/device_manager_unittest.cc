// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/chromedriver/chrome/adb.h"
#include "xwalk/test/chromedriver/chrome/device_manager.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class FakeAdb : public Adb {
 public:
  FakeAdb() {}
  virtual ~FakeAdb() {}

  virtual Status GetDevices(std::vector<std::string>* devices) OVERRIDE {
    devices->push_back("a");
    devices->push_back("b");
    return Status(kOk);
  }

  virtual Status ForwardPort(const std::string& device_serial,
                             int local_port,
                             const std::string& remote_abstract) OVERRIDE {
    return Status(kOk);
  }

  virtual Status SetCommandLineFile(const std::string& device_serial,
                                    const std::string& command_line_file,
                                    const std::string& exec_name,
                                    const std::string& args) OVERRIDE {
    return Status(kOk);
  }

  virtual Status CheckAppInstalled(const std::string& device_serial,
                                   const std::string& package) OVERRIDE {
    return Status(kOk);
  }

  virtual Status ClearAppData(const std::string& device_serial,
                              const std::string& package) OVERRIDE {
    return Status(kOk);
  }

  virtual Status Launch(const std::string& device_serial,
                        const std::string& package,
                        const std::string& activity) OVERRIDE {
    return Status(kOk);
  }

  virtual Status ForceStop(const std::string& device_serial,
                           const std::string& package) OVERRIDE {
    return Status(kOk);
  }

  virtual Status GetPidByName(const std::string& device_serial,
                              const std::string& process_name,
                              int* pid) OVERRIDE {
    return Status(kOk);
  }
};

}  // namespace

TEST(DeviceManager, AcquireDevice) {
  FakeAdb adb;
  DeviceManager device_manager(&adb);
  scoped_ptr<Device> device1;
  scoped_ptr<Device> device2;
  scoped_ptr<Device> device3;
  ASSERT_TRUE(device_manager.AcquireDevice(&device1).IsOk());
  ASSERT_TRUE(device_manager.AcquireDevice(&device2).IsOk());
  ASSERT_FALSE(device_manager.AcquireDevice(&device3).IsOk());
  device1.reset(NULL);
  ASSERT_TRUE(device_manager.AcquireDevice(&device3).IsOk());
  ASSERT_FALSE(device_manager.AcquireDevice(&device1).IsOk());
}

TEST(DeviceManager, AcquireSpecificDevice) {
  FakeAdb adb;
  DeviceManager device_manager(&adb);
  scoped_ptr<Device> device1;
  scoped_ptr<Device> device2;
  scoped_ptr<Device> device3;
  ASSERT_TRUE(device_manager.AcquireSpecificDevice("a", &device1).IsOk());
  ASSERT_FALSE(device_manager.AcquireSpecificDevice("a", &device2).IsOk());
  ASSERT_TRUE(device_manager.AcquireSpecificDevice("b", &device3).IsOk());
  device1.reset(NULL);
  ASSERT_TRUE(device_manager.AcquireSpecificDevice("a", &device2).IsOk());
  ASSERT_FALSE(device_manager.AcquireSpecificDevice("a", &device1).IsOk());
  ASSERT_FALSE(device_manager.AcquireSpecificDevice("b", &device1).IsOk());
}

TEST(Device, StartStopApp) {
  FakeAdb adb;
  DeviceManager device_manager(&adb);
  scoped_ptr<Device> device1;
  ASSERT_TRUE(device_manager.AcquireDevice(&device1).IsOk());
  ASSERT_TRUE(device1->StopApp().IsOk());
  ASSERT_TRUE(device1->StartApp("a.chrome.package", "", "", "", 0).IsOk());
  ASSERT_FALSE(device1->StartApp("a.chrome.package", "", "", "", 0).IsOk());
  ASSERT_TRUE(device1->StopApp().IsOk());
  ASSERT_FALSE(device1->StartApp(
      "a.chrome.package", "an.activity", "", "", 0).IsOk());
  ASSERT_FALSE(device1->StartApp("a.package", "", "", "", 0).IsOk());
  ASSERT_TRUE(device1->StartApp("a.package", "an.activity", "", "", 0).IsOk());
  ASSERT_TRUE(device1->StopApp().IsOk());
  ASSERT_TRUE(device1->StopApp().IsOk());
  ASSERT_TRUE(device1->StartApp(
      "a.package", "an.activity", "a.process", "", 0).IsOk());
  ASSERT_TRUE(device1->StopApp().IsOk());
}
