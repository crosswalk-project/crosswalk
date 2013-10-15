// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_COMMON_COMMON_API_BROWSERTEST_H_
#define XWALK_SYSAPPS_COMMON_COMMON_API_BROWSERTEST_H_

#include <string>
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/sysapps/common/binding_object_store.h"
#include "xwalk/sysapps/common/event_target.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionFunctionHandler;
using xwalk::extensions::XWalkExtensionFunctionInfo;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::sysapps::BindingObjectStore;
using xwalk::sysapps::EventTarget;

class SysAppsTestExtension : public XWalkExtension {
 public:
  SysAppsTestExtension();

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;
};

class SysAppsTestExtensionInstance : public XWalkExtensionInstance {
 public:
  SysAppsTestExtensionInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  void OnSysAppsTestObjectContructor(
      scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnHasObject(scoped_ptr<XWalkExtensionFunctionInfo> info);

  XWalkExtensionFunctionHandler handler_;
  BindingObjectStore store_;
};

class SysAppsTestObject : public EventTarget {
 public:
  SysAppsTestObject();

 private:
  // EventTarget implementation.
  virtual void StartEvent(const std::string& type) OVERRIDE;
  virtual void StopEvent(const std::string& type) OVERRIDE;

  // JavaScript function handlers.
  void OnIsTestEventActive(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnFireTestEvent(scoped_ptr<XWalkExtensionFunctionInfo> info);

  bool is_test_event_active_;
};

#endif  // XWALK_SYSAPPS_COMMON_COMMON_API_BROWSERTEST_H_
