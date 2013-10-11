// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_
#define XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_

#include <utility>
#include <string>
#include <vector>
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

using xwalk::extensions::XWalkExtensionFunctionHandler;
using xwalk::extensions::XWalkExtensionFunctionInfo;

class TestExtension : public xwalk::extensions::XWalkExtension {
 public:
  TestExtension();

  virtual const char* GetJavaScriptAPI() OVERRIDE;

  virtual xwalk::extensions::XWalkExtensionInstance* CreateInstance() OVERRIDE;
};

class TestExtensionInstance
    : public xwalk::extensions::XWalkExtensionInstance {
 public:
  typedef std::vector<std::pair<std::string, int> > Database;

  TestExtensionInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  Database* database() { return &database_; }

 private:
  void OnClearDatabase(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnAddPerson(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnAddPersonObject(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetAllPersons(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetPersonAge(scoped_ptr<XWalkExtensionFunctionInfo> info);

  std::vector<std::pair<std::string, int> > database_;

  XWalkExtensionFunctionHandler handler_;
};

#endif  // XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_
