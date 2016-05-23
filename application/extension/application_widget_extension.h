// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_EXTENSION_H_
#define XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_EXTENSION_H_

#include <string>

#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace application {
class Application;

using extensions::XWalkExtension;
using extensions::XWalkExtensionInstance;

class ApplicationWidgetExtension : public XWalkExtension {
 public:
  explicit ApplicationWidgetExtension(Application* application);

  // XWalkExtension implementation.
  XWalkExtensionInstance* CreateInstance() override;

 private:
  Application* application_;
};

class AppWidgetExtensionInstance : public XWalkExtensionInstance {
 public:
  explicit AppWidgetExtensionInstance(Application* application);
  ~AppWidgetExtensionInstance() override;

  void HandleMessage(std::unique_ptr<base::Value> msg) override;
  void HandleSyncMessage(std::unique_ptr<base::Value> msg) override;

 private:
  std::unique_ptr<base::StringValue> GetWidgetInfo(std::unique_ptr<base::Value> msg);
  std::unique_ptr<base::FundamentalValue> SetPreferencesItem(
      std::unique_ptr<base::Value> mgs);
  std::unique_ptr<base::FundamentalValue> RemovePreferencesItem(
      std::unique_ptr<base::Value> mgs);
  std::unique_ptr<base::FundamentalValue> ClearAllItems(std::unique_ptr<base::Value> mgs);
  std::unique_ptr<base::DictionaryValue> GetAllItems(std::unique_ptr<base::Value> mgs);
  std::unique_ptr<base::StringValue> GetItemValueByKey(std::unique_ptr<base::Value> mgs);
  std::unique_ptr<base::FundamentalValue> KeyExists(
      std::unique_ptr<base::Value> mgs) const;
  void PostMessageToOtherFrames(std::unique_ptr<base::DictionaryValue> msg);

  Application* application_;
  std::unique_ptr<class AppWidgetStorage> widget_storage_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_EXTENSION_H_
