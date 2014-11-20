// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APPWIDGET_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APPWIDGET_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

typedef std::map<std::string, std::string> TizenAppWidgetLabelLangValueMap;

struct TizenAppWidgetLabel {
  // may be empty
  std::string default_value;

  // may be empty if default is set
  TizenAppWidgetLabelLangValueMap lang_value_map;
};

enum TizenAppWidgetSizeType {
  k1x1, k2x1, k2x2
};

struct TizenAppWidgetSize {
  // mandatory
  TizenAppWidgetSizeType type;

  // optional, relative to web app directory
  std::string preview;

  // optional, default: true
  bool use_decoration;
};

typedef std::vector<TizenAppWidgetSize> TizenAppWidgetSizeVector;

struct TizenAppWidgetDropView {
  // mandatory, relative to web app directory or remote URL
  std::string src;

  // mandatory
  int width;

  // mandatory, <1, 380>
  int height;
};

typedef std::vector<TizenAppWidgetDropView> TizenAppWidgetDropViewVector;

struct TizenAppWidget {
  // mandatory, unique, must start with application id and end with label
  // separated with dot, the label can contain only 0-9, a-z, A-Z
  std::string id;

  // mandatory, if 2 or more app widgets have the primary attribute set to true,
  // the default icon and title of the parent web app can be used
  bool primary;

  // optional(0-1), min: 1800.0, default: no update
  std::vector<double> update_period;

  // optional, default: false
  bool auto_launch;

  // box label, multiple(1+)
  TizenAppWidgetLabel label;

  // box icon, optional(0-1), src, mandatory, relative to web app directory
  std::string icon_src;

  // box content, mandatory(1) -[

  // mandatory, relative to web app directory or remote URL
  std::string content_src;

  // optional, default: false
  bool content_mouse_event;

  // optional, default: true
  bool content_touch_effect;

  // box size, mandatory(1-3), 1x1 must exist
  TizenAppWidgetSizeVector content_size;

  // drop view, optional(0-1)
  TizenAppWidgetDropViewVector content_drop_view;

  // ]- box content
};

typedef std::vector<TizenAppWidget> TizenAppWidgetVector;

class TizenAppWidgetInfo : public ApplicationData::ManifestData {
 public:
  explicit TizenAppWidgetInfo(const TizenAppWidgetVector& app_widgets);
  virtual ~TizenAppWidgetInfo();

  const TizenAppWidgetVector& app_widgets() const {
    return app_widgets_;
  }

 private:
  // multiple(0+)
  TizenAppWidgetVector app_widgets_;
};

class TizenAppWidgetHandler : public ManifestHandler {
 public:
  TizenAppWidgetHandler();
  virtual ~TizenAppWidgetHandler();

  bool Parse(scoped_refptr<ApplicationData> application,
             base::string16* error) override;
  bool Validate(scoped_refptr<const ApplicationData> application,
                std::string* error) const override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenAppWidgetHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APPWIDGET_HANDLER_H_
