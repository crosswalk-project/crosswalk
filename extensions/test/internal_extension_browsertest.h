// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_
#define XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_

#include <utility>
#include <string>
#include <vector>
#include "xwalk/extensions/browser/xwalk_extension_internal.h"

using xwalk::extensions::XWalkExtensionFunctionHandler;

class TestExtension : public xwalk::extensions::XWalkInternalExtension {
 public:
  TestExtension();

  virtual const char* GetJavaScriptAPI() OVERRIDE;

  virtual xwalk::extensions::XWalkExtensionInstance* CreateInstance(
      const XWalkExtension::PostMessageCallback& post_message) OVERRIDE;
};

class TestExtensionInstance
    : public xwalk::extensions::XWalkInternalExtensionInstance {
 public:
  typedef std::vector<std::pair<std::string, int> > Database;

  TestExtensionInstance(
      const xwalk::extensions::XWalkExtension::PostMessageCallback&
          post_message);

  Database* database() { return &database_; }

 private:
  void OnClearDatabase(const FunctionInfo& info);
  void OnAddPerson(const FunctionInfo& info);
  void OnAddPersonObject(const FunctionInfo& info);
  void OnGetAllPersons(const FunctionInfo& info);
  void OnGetPersonAge(const FunctionInfo& info);

  std::vector<std::pair<std::string, int> > database_;
};

#endif  // XWALK_EXTENSIONS_TEST_INTERNAL_EXTENSION_BROWSERTEST_H_
