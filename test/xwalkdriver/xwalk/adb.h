// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_ADB_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_ADB_H_

#include <string>
#include <vector>

class Status;

class Adb {
 public:
  virtual ~Adb() {}

  virtual Status GetDevices(std::vector<std::string>* devices) = 0;
  virtual Status ForwardPort(const std::string& device_serial,
                             int local_port,
                             const std::string& remote_abstract) = 0;
  virtual Status SetCommandLineFile(const std::string& device_serial,
                                    const std::string& command_line_file,
                                    const std::string& exec_name,
                                    const std::string& args) = 0;
  virtual Status CheckAppInstalled(const std::string& device_serial,
                                   const std::string& package) = 0;
  virtual Status ClearAppData(const std::string& device_serial,
                              const std::string& package) = 0;
  virtual Status Launch(const std::string& device_serial,
                        const std::string& package,
                        const std::string& activity) = 0;
  virtual Status ForceStop(const std::string& device_serial,
                           const std::string& package) = 0;
  virtual Status GetPidByName(const std::string& device_serial,
                              const std::string& process_name,
                              int* pid) = 0;
};

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_ADB_H_
