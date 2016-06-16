// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_
#define XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_

#include <utility>
#include <string>
#include <vector>
#include "base/timer/timer.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

using xwalk::extensions::XWalkExtensionFunctionHandler;
using xwalk::extensions::XWalkExtensionFunctionInfo;

class TestExtension : public xwalk::extensions::XWalkExtension {
 public:
  TestExtension();

  xwalk::extensions::XWalkExtensionInstance* CreateInstance() override;
};

class TestExtensionInstance
    : public xwalk::extensions::XWalkExtensionInstance {
 public:
  typedef std::vector<std::pair<std::string, int> > Database;

  TestExtensionInstance();
  ~TestExtensionInstance() override;

  void HandleMessage(std::unique_ptr<base::Value> msg) override;

  Database* database() { return &database_; }

 private:
  void OnClearDatabase(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnAddPerson(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnAddPersonObject(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetAllPersons(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetPersonAge(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnStartHeartbeat(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnStopHeartbeat(std::unique_ptr<XWalkExtensionFunctionInfo> info);

  void DispatchHeartbeat();

  std::vector<std::pair<std::string, int> > database_;

  int counter_;
  std::unique_ptr<XWalkExtensionFunctionInfo> heartbeat_info_;
  base::RepeatingTimer timer_;

  XWalkExtensionFunctionHandler handler_;
};

#endif  // XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_
