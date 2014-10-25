// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_appwidget_handler.h"

#include <limits>
#include <set>

#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "third_party/re2/re2/re2.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/manifest_handlers_util.h"
#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {

const char kErrMsgMultipleKeys[] =
    "Too many keys found. Key name: ";
const char kErrMsgAppWidgetInfoNotFound[] =
    "Cannot access app-widget info object.";
const char kErrMsgApplicationInfoNotFound[] =
    "Cannot access application info object.";
const char kErrMsgDuplicatedAppWidgetId[] =
    "Duplicated value of an id attribute in app-widget element. The value: ";
const char kErrMsgInvalidAppWidgetIdBeginning[] =
    "Invalid beginning of an id attribute value in app-widget element."
    " The value: ";
const char kErrMsgInvalidAppWidgetIdFormat[] =
    "Invalid format of an id attribute value in app-widget element."
    " The value: ";
const char kErrMsgNoPrimaryAppWidget[] =
    "No primary app-widget element (primary='true').";
const char kErrMsgToManyPrimaryAppWidgets[] =
    "Too many primary app-widget elements (primary='true').";
const char kErrMsgUpdatePeriodOutOfDomain[] =
    "Value of an update-period attribute in app-widget element out of domain."
    " The value: ";
const char kErrMsgNoLabel[] =
    "No box-label element in app-widget element.";
const char kErrMsgInvalidIconSrc[] =
    "Invalid path in a src attribute of box-icon element. The value: ";
const char kErrMsgInvalidContentSrc[] =
    "Invalid path or url in a src attribute of box-content element."
    " The value: ";
const char kErrMsgInvalidContentSizePreview[] =
    "Invalid path in a preview attribute of box-size element. The value: ";
const char kErrMsgNoMandatoryContentSize1x1[] =
    "No mandatory box-size element (1x1) in box-content element.";
const char kErrMsgInvalidContentDropViewSrc[] =
    "Invalid path or url in a src attribute of pd element. The value: ";
const char kErrMsgContentDropViewHeightOutOfDomain[] =
    "Value of a height attribute in box-content element out of domain."
    " The value: ";

// Parses box-label part
bool ParseLabel(const base::DictionaryValue& dict,
    const std::string& key, TizenAppWidget* app_widget, base::string16* error) {
  DCHECK(app_widget);

  std::string lang;
  if (!GetOptionalValue(dict, keys::kTizenAppWidgetBoxLabelLangKey,
      std::string(), &lang, error))
    return false;

  std::string text;
  if (!GetMandatoryValue(dict, keys::kTizenAppWidgetBoxLabelTextKey,
      &text, error))
    return false;

  if (lang.empty()) {
    // Note: Tizen 2.2 WRT Core Spec does not determine how many times the value
    // without lang attribute can appear in one app-widget, so overwrite.
    app_widget->label.default_value = text;
  } else {
    // Note: Tizen 2.2 WRT Core Spec does not determine how many times the value
    // with specific lang attribute can appear in one app-widget, so overwrite.
    app_widget->label.lang_value_map[lang] = text;
  }

  return true;
}

// Parses box-icon part
bool ParseIcon(const base::DictionaryValue& dict,
    const std::string& key, TizenAppWidget* app_widget, base::string16* error) {
  DCHECK(app_widget);

  if (!app_widget->icon_src.empty()) {
    SafeSetError(kErrMsgMultipleKeys, key, error);
    return false;
  }
  if (!GetMandatoryValue(dict, keys::kTizenAppWidgetBoxIconSrcKey,
      &app_widget->icon_src, error))
    return false;

  return true;
}

// Converts size type from text to enum representation
bool StringToSizeType(const std::string& str_type,
    TizenAppWidgetSizeType* enum_type) {
  DCHECK(enum_type);
  if (str_type == "1x1") {
    *enum_type = TizenAppWidgetSizeType::k1x1;
    return true;
  }
  if (str_type == "2x1") {
    *enum_type = TizenAppWidgetSizeType::k2x1;
    return true;
  }
  if (str_type == "2x2") {
    *enum_type = TizenAppWidgetSizeType::k2x2;
    return true;
  }
  return false;
}

// Parses box-size part
bool ParseContentSizes(const base::DictionaryValue& dict,
    const std::string& key, TizenAppWidget* app_widget, base::string16* error) {
  DCHECK(app_widget);

  TizenAppWidgetSize size;

  std::string str_type;
  if (!GetMandatoryValue(dict, keys::kTizenAppWidgetBoxContentSizeTextKey,
      &str_type, error))
    return false;

  TizenAppWidgetSizeType type;
  if (!StringToSizeType(str_type, &type)) {
    SafeSetError(kErrMsgInvalidKeyValue,
        keys::kTizenAppWidgetBoxContentSizeTextKey, error);
    return false;
  }
  size.type = type;

  if (!GetOptionalValue(dict, keys::kTizenAppWidgetBoxContentSizePreviewKey,
      std::string(), &size.preview, error))
    return false;

  if (!GetOptionalValue(dict,
      keys::kTizenAppWidgetBoxContentSizeUseDecorationKey,
      true, &size.use_decoration, error))
    return false;

  app_widget->content_size.push_back(size);

  return true;
}

// Parses pd part
bool ParseContentDropView(const base::DictionaryValue& dict,
    const std::string& key, TizenAppWidget* app_widget, base::string16* error) {
  DCHECK(app_widget);

  if (!app_widget->content_drop_view.empty()) {
    SafeSetError(kErrMsgMultipleKeys, key, error);
    return false;
  }

  TizenAppWidgetDropView drop_view;

  if (!GetMandatoryValue(dict, keys::kTizenAppWidgetBoxContentDropViewSrcKey,
      &drop_view.src, error))
    return false;

  if (!GetMandatoryValue(dict,
      keys::kTizenAppWidgetBoxContentDropViewWidthKey,
      &drop_view.width, error))
    return false;

  if (!GetMandatoryValue(dict,
      keys::kTizenAppWidgetBoxContentDropViewHeightKey,
      &drop_view.height, error))
    return false;

  app_widget->content_drop_view.push_back(drop_view);

  return true;
}

// Parses box-content part
bool ParseContent(const base::DictionaryValue& dict,
    const std::string& key, TizenAppWidget* app_widget, base::string16* error) {
  DCHECK(app_widget);

  if (!app_widget->content_src.empty()) {
    SafeSetError(kErrMsgMultipleKeys, key, error);
    return false;
  }
  if (!GetMandatoryValue(dict, keys::kTizenAppWidgetBoxContentSrcKey,
      &app_widget->content_src, error))
    return false;

  if (!GetOptionalValue(dict, keys::kTizenAppWidgetBoxContentMouseEventKey,
      false, &app_widget->content_mouse_event, error))
    return false;

  if (!GetOptionalValue(dict, keys::kTizenAppWidgetBoxContentTouchEffectKey,
      true, &app_widget->content_touch_effect, error))
    return false;

  if (!ForEachDictionaryCall(dict, keys::kTizenAppWidgetBoxContentSizeKey,
      ValidCount::Min(1), ParseContentSizes, app_widget, error))
    return false;

  if (!ForEachDictionaryCall(dict, keys::kTizenAppWidgetBoxContentDropViewKey,
      ValidCount::Max(1), ParseContentDropView, app_widget, error))
    return false;

  return true;
}

// Parses app-widget part
bool ParseAppWidget(const base::DictionaryValue& dict,
    const std::string& key, TizenAppWidgetVector* app_widgets,
    base::string16* error) {
  DCHECK(app_widgets);

  TizenAppWidget app_widget;

  if (!GetMandatoryValue(dict, keys::kTizenAppWidgetIdKey,
      &app_widget.id, error))
    return false;

  if (!GetMandatoryValue(dict, keys::kTizenAppWidgetPrimaryKey,
      &app_widget.primary, error))
    return false;

  double update_period;
  double no_update_period = std::numeric_limits<double>::min();
  if (!GetOptionalValue(dict, keys::kTizenAppWidgetUpdatePeriodKey,
      no_update_period, &update_period, error))
    return false;
  if (update_period != no_update_period)
    app_widget.update_period.push_back(update_period);

  if (!GetOptionalValue(dict, keys::kTizenAppWidgetAutoLaunchKey,
      false, &app_widget.auto_launch, error))
    return false;

  if (!ForEachDictionaryCall(dict, keys::kTizenAppWidgetBoxLabelKey,
      ValidCount::Min(1), ParseLabel, &app_widget, error))
    return false;

  if (!ForEachDictionaryCall(dict, keys::kTizenAppWidgetBoxIconKey,
      ValidCount::Max(1), ParseIcon, &app_widget, error))
    return false;

  if (!ForEachDictionaryCall(dict, keys::kTizenAppWidgetBoxContentKey,
      ValidCount::Exactly(1), ParseContent, &app_widget, error))
    return false;

  app_widgets->push_back(app_widget);

  return true;
}

// Validates all app-widget ids
bool ValidateEachId(const TizenAppWidgetVector& app_widgets,
    const std::string& app_id, std::string* error) {
  std::set<std::string> unique_values;

  for (const TizenAppWidget& app_widget : app_widgets) {
    if (!unique_values.insert(app_widget.id).second) {
      SafeSetError(kErrMsgDuplicatedAppWidgetId, app_widget.id, error);
      return false;
    }

    const size_t app_id_len = app_id.length();

    if (app_widget.id.find(app_id) != 0) {
      SafeSetError(kErrMsgInvalidAppWidgetIdBeginning, app_widget.id, error);
      return false;
    }

    const char kStringPattern[] = "[.][0-9a-zA-Z]+";
    if (!RE2::FullMatch(app_widget.id.substr(app_id_len), kStringPattern)) {
      SafeSetError(kErrMsgInvalidAppWidgetIdFormat, app_widget.id, error);
      return false;
    }
  }

  return true;
}

// Validates all app-widget primary attributes
bool ValidateEachPrimary(const TizenAppWidgetVector& app_widgets,
    std::string* error) {
  int primary_count = 0;

  for (const TizenAppWidget& app_widget : app_widgets)
    if (app_widget.primary)
      ++primary_count;

  if (!primary_count) {
    SafeSetError(kErrMsgNoPrimaryAppWidget, error);
    return false;
  }

  if (primary_count > 1) {
    SafeSetError(kErrMsgToManyPrimaryAppWidgets, error);
    return false;
  }

  return true;
}

// Tests if specified string represents valid remote url
bool IsValidUrl(const std::string& value) {
  // TODO(tweglarski): implement me (it's not crucial atm)
  return true;
}

// Tests if specified string represents valid path
bool IsValidPath(const std::string& value) {
  // TODO(tweglarski): implement me (it's not crucial atm)
  return true;
}

// Tests if specified string represents valid path or remote url
bool IsValidPathOrUrl(const std::string& value) {
  return IsValidPath(value) || IsValidUrl(value);
}

// Validates all content sizes in an app-widget
bool ValidateContentSize(const TizenAppWidgetSizeVector& content_size,
    std::string* error) {
  bool mandatory_1x1_found = false;

  for (const TizenAppWidgetSize& size : content_size) {
    mandatory_1x1_found |= size.type == TizenAppWidgetSizeType::k1x1;

    if (!size.preview.empty() && !IsValidPath(size.preview)) {
      SafeSetError(kErrMsgInvalidContentSizePreview, size.preview, error);
      return false;
    }
  }

  if (!mandatory_1x1_found) {
    SafeSetError(kErrMsgNoMandatoryContentSize1x1, error);
    return false;
  }

  return true;
}

}  // namespace

TizenAppWidgetInfo::TizenAppWidgetInfo(const TizenAppWidgetVector& app_widgets)
    : app_widgets_(app_widgets) {
}

TizenAppWidgetInfo::~TizenAppWidgetInfo() {
}

TizenAppWidgetHandler::TizenAppWidgetHandler() {
}

TizenAppWidgetHandler::~TizenAppWidgetHandler() {
}

bool TizenAppWidgetHandler::Parse(scoped_refptr<ApplicationData> application,
    base::string16* error) {
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);

  const base::DictionaryValue* dict = nullptr;
  if (!GetMandatoryDictionary(*manifest, keys::kTizenWidgetKey, &dict, error))
    return false;

  TizenAppWidgetVector app_widgets;

  if (!ForEachDictionaryCall(*dict, keys::kTizenAppWidgetKey,
      ValidCount::Any(), ParseAppWidget, &app_widgets, error))
    return false;

  scoped_ptr<TizenAppWidgetInfo> info(new TizenAppWidgetInfo(app_widgets));
  application->SetManifestData(keys::kTizenAppWidgetFullKey, info.release());

  return true;
}

bool TizenAppWidgetHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error) const {
  const TizenAppWidgetInfo* app_widget_info =
      static_cast<const TizenAppWidgetInfo*>(
          application->GetManifestData(keys::kTizenAppWidgetFullKey));
  const TizenApplicationInfo* app_info =
      static_cast<const TizenApplicationInfo*>(
          application->GetManifestData(keys::kTizenApplicationKey));

  if (!app_widget_info) {
    SafeSetError(kErrMsgAppWidgetInfoNotFound, error);
    return false;
  }
  if (!app_info) {
    SafeSetError(kErrMsgApplicationInfoNotFound, error);
    return false;
  }

  const TizenAppWidgetVector& app_widgets = app_widget_info->app_widgets();

  if (!ValidateEachId(app_widgets, app_info->id(), error))
    return false;

  if (!ValidateEachPrimary(app_widgets, error))
    return false;

  for (const TizenAppWidget& app_widget : app_widgets) {
    if (!app_widget.update_period.empty()
        && app_widget.update_period.front() < 1800) {
      SafeSetError(kErrMsgUpdatePeriodOutOfDomain,
          base::DoubleToString(app_widget.update_period.front()), error);
      return false;
    }

    if (app_widget.label.default_value.empty()
        && app_widget.label.lang_value_map.empty()) {
      SafeSetError(kErrMsgNoLabel, error);
      return false;
    }

    if (!app_widget.icon_src.empty()
        && !IsValidPathOrUrl(app_widget.icon_src)) {
      SafeSetError(kErrMsgInvalidIconSrc, app_widget.icon_src, error);
      return false;
    }

    if (!IsValidPathOrUrl(app_widget.content_src)) {
      SafeSetError(kErrMsgInvalidContentSrc, app_widget.content_src, error);
      return false;
    }

    if (!ValidateContentSize(app_widget.content_size, error))
      return false;

    if (!app_widget.content_drop_view.empty()) {
      const TizenAppWidgetDropView& drop_view
          = app_widget.content_drop_view.front();

      if (!IsValidPathOrUrl(drop_view.src)) {
        SafeSetError(kErrMsgInvalidContentDropViewSrc, drop_view.src, error);
        return false;
      }

      if (drop_view.height < 1 || drop_view.height > 380) {
        SafeSetError(kErrMsgContentDropViewHeightOutOfDomain,
            base::IntToString(drop_view.height), error);
        return false;
      }
    }
  }

  return true;
}

std::vector<std::string> TizenAppWidgetHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenAppWidgetFullKey);
}

}  // namespace application
}  // namespace xwalk
