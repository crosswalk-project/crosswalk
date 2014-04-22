// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/widget_handler.h"

#include <map>
#include <utility>
#include <vector>

#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace {
// Below key names are readable from Javascript widget interface.
const char kAuthor[] = "author";
const char kDecription[] = "description";
const char kName[] = "name";
const char kShortName[] = "shortName";
const char kVersion[] = "version";
const char kID[] = "id";
const char kAuthorEmail[] = "authorEmail";
const char kAuthorHref[] = "authorHref";
const char kHeight[] = "height";
const char kWidth[] = "width";
const char kPreferences[] = "preferences";

// Child keys inside 'preferences' key.
const char kPreferencesName[] = "name";
const char kPreferencesValue[] = "value";
const char kPreferencesReadonly[] = "readonly";

typedef std::map<std::string, std::string> KeyMap;
typedef std::map<std::string, std::string>::const_iterator KeyMapIterator;
typedef std::pair<std::string, std::string> KeyPair;

const std::string GetKeyWithPath(const std::string& path,
                                 const std::string& key) {
  std::string path_key(path);
  path_key.append(".");
  path_key.append(key);
  return path_key;
}

const KeyMap& GetWidgetKeyPairs() {
  static KeyMap map;
  if (map.empty()) {
    map.insert(KeyPair(keys::kAuthorKey, kAuthor));
    map.insert(KeyPair(keys::kVersionKey, kVersion));
    map.insert(KeyPair(keys::kIDKey, kID));
    map.insert(KeyPair(keys::kAuthorEmailKey, kAuthorEmail));
    map.insert(KeyPair(keys::kAuthorHrefKey, kAuthorHref));
    map.insert(KeyPair(keys::kHeightKey, kHeight));
    map.insert(KeyPair(keys::kWidthKey, kWidth));
  }

  return map;
}

void ParsePreferenceItem(const base::DictionaryValue* in_value,
                         base::DictionaryValue* out_value) {
  DCHECK(in_value && in_value->IsType(base::Value::TYPE_DICTIONARY));

  std::string pref_name;
  std::string pref_value;
  std::string pref_readonly;
  if (in_value->GetString(keys::kPreferencesNameKey, &pref_name))
    out_value->SetString(kPreferencesName, pref_name);

  if (in_value->GetString(keys::kPreferencesValueKey, &pref_value))
    out_value->SetString(kPreferencesValue, pref_value);

  if (in_value->GetString(keys::kPreferencesReadonlyKey, &pref_readonly)) {
      out_value->SetBoolean(kPreferencesReadonly, pref_readonly == "true");
  }
}

}  // namespace

namespace application {

WidgetInfo::WidgetInfo()
    : value_(new base::DictionaryValue),
      manifest_i18n_data_(NULL) {}

WidgetInfo::~WidgetInfo() {}

void WidgetInfo::SetString(const std::string& key, const std::string& value) {
  value_->SetString(key, value);
}

void WidgetInfo::Set(const std::string& key, base::Value* value) {
  value_->Set(key, value);
}

base::DictionaryValue* WidgetInfo::GetWidgetInfo() {
  DCHECK(manifest_i18n_data_);
  std::string name;
  std::string shortName;
  std::string description;

  manifest_i18n_data_->GetString(
      GetKeyWithPath(keys::kNameKey, keys::kTextKey), &name);
  manifest_i18n_data_->GetString(
      GetKeyWithPath(keys::kNameKey, keys::kNameShortNameKey), &shortName);
  manifest_i18n_data_->GetString(
      GetKeyWithPath(keys::kDescriptionKey, keys::kTextKey), &description);
  value_->SetString(kName, name);
  value_->SetString(kShortName, shortName);
  value_->SetString(kDecription, description);

  return value_.get();
}

WidgetHandler::WidgetHandler() {}

WidgetHandler::~WidgetHandler() {}

bool WidgetHandler::Parse(scoped_refptr<ApplicationData> application,
                          base::string16* error) {
  scoped_ptr<WidgetInfo> widget_info(new WidgetInfo);
  const Manifest* manifest = application->GetManifest();
  ManifestI18NData* manifest_i18n_data = application->GetManifestI18NData();
  DCHECK(manifest);
  DCHECK(manifest_i18n_data);

  std::string default_locale;
  manifest->GetString(keys::kDefaultLocaleKey, &default_locale);
  // The defaultlocale is an attribute of widget element,
  // so only widget handler need to set defaultlocale
  manifest_i18n_data->SetDefaultLocale(default_locale);

  const KeyMap& map = GetWidgetKeyPairs();

  for (KeyMapIterator iter = map.begin(); iter != map.end(); ++iter) {
    std::string string;
    manifest->GetString(iter->first, &string);
    widget_info->SetString(iter->second, string);
  }

  manifest_i18n_data->ParseFromPath(manifest, keys::kNameKey);
  manifest_i18n_data->ParseFromPath(manifest, keys::kDescriptionKey);

  base::Value* pref_value = NULL;
  manifest->Get(keys::kPreferencesKey, &pref_value);

  if (pref_value && pref_value->IsType(base::Value::TYPE_DICTIONARY)) {
    base::DictionaryValue* preferences = new base::DictionaryValue;
    base::DictionaryValue* dict;
    pref_value->GetAsDictionary(&dict);
    ParsePreferenceItem(dict, preferences);
    widget_info->Set(kPreferences, preferences);
  } else if (pref_value && pref_value->IsType(base::Value::TYPE_LIST)) {
    base::ListValue* preferences = new base::ListValue;
    base::ListValue* list;
    pref_value->GetAsList(&list);

    for (base::ListValue::iterator it = list->begin();
         it != list->end(); ++it) {
      base::DictionaryValue* pref = new base::DictionaryValue;
      base::DictionaryValue* dict;
      (*it)->GetAsDictionary(&dict);
      ParsePreferenceItem(dict, pref);
      preferences->Append(pref);
    }
    widget_info->Set(kPreferences, preferences);
  }

  widget_info->SetManifestI18NData(manifest_i18n_data);
  application->SetManifestData(keys::kWidgetKey, widget_info.release());
  return true;
}

bool WidgetHandler::AlwaysParseForType(Manifest::Type type) const {
  return true;
}

std::vector<std::string> WidgetHandler::Keys() const {
  return std::vector<std::string>(1, keys::kWidgetKey);
}

}  // namespace application
}  // namespace xwalk
