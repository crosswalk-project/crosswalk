// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_EXTENSION_H_
#define XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_EXTENSION_H_

#include <string>

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace application {
class Application;

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class ApplicationWidgetExtension : public XWalkExtension {
 public:
  explicit ApplicationWidgetExtension(Application* application);

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

 private:
  Application* application_;
};

class AppWidgetExtensionInstance : public XWalkExtensionInstance {
 public:
  explicit AppWidgetExtensionInstance(Application* application);
  virtual ~AppWidgetExtensionInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  scoped_ptr<base::StringValue> GetWidgetInfo(scoped_ptr<base::Value> msg);
  scoped_ptr<base::FundamentalValue> SetPreferencesItem(
      scoped_ptr<base::Value> mgs);
  scoped_ptr<base::FundamentalValue> RemovePreferencesItem(
      scoped_ptr<base::Value> mgs);
  scoped_ptr<base::FundamentalValue> ClearAllItems(scoped_ptr<base::Value> mgs);
  scoped_ptr<base::DictionaryValue> GetAllItems(scoped_ptr<base::Value> mgs);
  scoped_ptr<base::FundamentalValue> KeyExists(
      scoped_ptr<base::Value> mgs) const;

  Application* application_;
  scoped_ptr<class AppWidgetStorage> widget_storage_;
  XWalkExtensionFunctionHandler handler_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_EXTENSION_H_
