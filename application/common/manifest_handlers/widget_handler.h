// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WIDGET_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WIDGET_HANDLER_H_

#include <map>
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
  virtual ~WidgetInfo();
  void SetString(const std::string& key, const std::string& value);
  void Set(const std::string& key, base::Value* value);

  base::DictionaryValue* GetWidgetInfo() {
    return value_.get();
  }

 private:
  scoped_ptr<base::DictionaryValue> value_;
};

class WidgetHandler : public ManifestHandler {
 public:
  WidgetHandler();
  virtual ~WidgetHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application,
                     base::string16* error) OVERRIDE;
  virtual bool Validate(scoped_refptr<const ApplicationData> application,
                        std::string* error,
                        std::vector<InstallWarning>* warnings) const OVERRIDE;
  virtual bool AlwaysParseForType(Manifest::Type type) const OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(WidgetHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WIDGET_HANDLER_H_
