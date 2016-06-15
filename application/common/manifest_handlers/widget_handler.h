// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WIDGET_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WIDGET_HANDLER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/values.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class WidgetInfo : public ApplicationData::ManifestData {
 public:
  WidgetInfo();
  ~WidgetInfo() override;
  void SetString(const std::string& key, const std::string& value);
  void Set(const std::string& key, std::unique_ptr<base::Value> value);

  // Name, shrot name and description are i18n items, they will be set
  // if their value were changed after loacle was changed.
  void SetName(const std::string& name);
  void SetShortName(const std::string& short_name);
  void SetDescription(const std::string& description);

  base::DictionaryValue* GetWidgetInfo() {
    return value_.get();
  }

 private:
  std::unique_ptr<base::DictionaryValue> value_;
};

class WidgetHandler : public ManifestHandler {
 public:
  WidgetHandler();
  ~WidgetHandler() override;

  bool Parse(scoped_refptr<ApplicationData> application,
             base::string16* error) override;
  bool AlwaysParseForType(Manifest::Type type) const override;
  std::vector<std::string> Keys() const override;

  bool Validate(scoped_refptr<const ApplicationData> application,
                std::string* error) const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(WidgetHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WIDGET_HANDLER_H_
