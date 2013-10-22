// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_EXTENSION_APPLICATION_EXTENSION_H_
#define XWALK_APPLICATION_EXTENSION_APPLICATION_EXTENSION_H_

#include <string>

#include "base/threading/thread_checker.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {

namespace application {
class ApplicationSystem;
}

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class ApplicationExtension : public XWalkExtension {
 public:
  explicit ApplicationExtension(
      application::ApplicationSystem* application_system);

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

 private:
  application::ApplicationSystem* application_system_;
};

class ApplicationExtensionInstance : public XWalkExtensionInstance {
 public:
  explicit ApplicationExtensionInstance(
      application::ApplicationSystem* application_system);

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  void OnGetMainDocumentID(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetManifest(scoped_ptr<XWalkExtensionFunctionInfo> info);


  // Get main document routing ID from ApplicationProcessManager on UI thread.
  void GetMainDocumentID(int* main_routing_id);
  // Post id back to renderer on extension thread.
  void PostMainDocumentID(scoped_ptr<XWalkExtensionFunctionInfo> info,
                          int* main_routing_id);
  // Copy manifest data on UI thread.
  void GetManifest(base::DictionaryValue** manifest_data);
  // Post dictionary value of manifest to renderer on extension thread.
  void PostManifest(scoped_ptr<XWalkExtensionFunctionInfo> info,
                    base::DictionaryValue** manifest_data);

  application::ApplicationSystem* application_system_;

  base::ThreadChecker thread_checker_;

  XWalkExtensionFunctionHandler handler_;
};

}  // namespace xwalk

#endif  // XWALK_APPLICATION_EXTENSION_APPLICATION_EXTENSION_H_
