// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_EXTENSION_APPLICATION_EVENT_EXTENSION_H_
#define XWALK_APPLICATION_EXTENSION_APPLICATION_EVENT_EXTENSION_H_

#include <map>
#include <string>

#include "xwalk/application/browser/event_observer.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {

namespace application {
class ApplicationEventManager;
class ApplicationSystem;
}

class AppEventExtensionInstance;

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class ApplicationEventExtension : public XWalkExtension {
 public:
  explicit ApplicationEventExtension(application::ApplicationSystem* system);

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

 private:
  application::ApplicationSystem* application_system_;
};

class AppEventExtensionInstance : public XWalkExtensionInstance,
                                  public application::EventObserver {
 public:
  AppEventExtensionInstance(application::ApplicationEventManager* event_manager,
                            const std::string& app_id);

  // XWalkExtensionInstance implementation.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  // EventObserver implementation.
  virtual void Observe(const std::string& app_id,
                       scoped_refptr<application::Event> event) OVERRIDE;

 private:
  // Registered handlers for incoming JS messages.
  void OnRegisterEvent(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnUnregisterEvent(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnDispatchEventFinish(scoped_ptr<XWalkExtensionFunctionInfo> info);

  typedef std::map<std::string, XWalkExtensionFunctionInfo::PostResultCallback>
      EventCallbackMap;
  EventCallbackMap registered_events_;
  std::string app_id_;

  XWalkExtensionFunctionHandler handler_;
};

}  // namespace xwalk

#endif  // XWALK_APPLICATION_EXTENSION_APPLICATION_EVENT_EXTENSION_H_
