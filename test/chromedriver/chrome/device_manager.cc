// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/chrome/device_manager.h"

#include <algorithm>
#include <vector>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "xwalk/test/chromedriver/chrome/adb.h"
#include "xwalk/test/chromedriver/chrome/status.h"

Device::Device(
    const std::string& device_serial, Adb* adb,
    base::Callback<void()> release_callback)
    : serial_(device_serial),
      adb_(adb),
      release_callback_(release_callback) {}

Device::~Device() {
  release_callback_.Run();
}

Status Device::StartApp(const std::string& package,
                        const std::string& activity,
                        const std::string& process,
                        const std::string& args,
                        int port) {
  // TODO(Peter Wang): Need to implement it for android xwalk.
  (void) package;
  (void) activity;
  (void) process;
  (void) args;
  (void) port;

  return Status(kOk);
}

Status Device::StopApp() {
  // TODO(Peter Wang): Need to implement it for android xwalk.

  return Status(kOk);
}

DeviceManager::DeviceManager(Adb* adb) : adb_(adb) {
  // TODO(Peter Wang): Need to implement it for android xwalk.
}

DeviceManager::~DeviceManager() {}

Status DeviceManager::AcquireDevice(scoped_ptr<Device>* device) {
  // TODO(Peter Wang): Need to implement it for android xwalk.
  (void) device;

  return Status(kOk);
}

Status DeviceManager::AcquireSpecificDevice(
    const std::string& device_serial, scoped_ptr<Device>* device) {
  // TODO(Peter Wang): Need to implement it for android xwalk.
  (void) device_serial;
  (void) device;

  return Status(kOk);
}

void DeviceManager::ReleaseDevice(const std::string& device_serial) {
  // TODO(Peter Wang): Need to implement it for android xwalk.
  (void) device_serial;
}

Device* DeviceManager::LockDevice(const std::string& device_serial) {
  // TODO(Peter Wang): Need to implement it for android xwalk.
  (void) device_serial;

  return NULL;
}

bool DeviceManager::IsDeviceLocked(const std::string& device_serial) {
  // TODO(Peter Wang): Need to implement it for android xwalk.
  (void) device_serial;

  return false;
}

