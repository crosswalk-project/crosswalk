// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/widget_handler.h"

#include <map>
#include <utility>
#include <set>

#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "xwalk/application/common/application_manifest_constants.h"

#if defined(OS_WIN)
#define strcasecmp _stricmp
#endif

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

const KeyMap& GetWidgetKeyPairs() {
  static KeyMap map;
  if (map.empty()) {
    map.insert(KeyPair(keys::kAuthorKey, kAuthor));
    map.insert(KeyPair(keys::kDescriptionKey, kDecription));
    map.insert(KeyPair(keys::kNameKey, kName));
    map.insert(KeyPair(keys::kShortNameKey, kShortName));
    map.insert(KeyPair(keys::kVersionKey, kVersion));
    map.insert(KeyPair(keys::kIDKey, kID));
    map.insert(KeyPair(keys::kAuthorEmailKey, kAuthorEmail));
    map.insert(KeyPair(keys::kAuthorHrefKey, kAuthorHref));
    map.insert(KeyPair(keys::kHeightKey, kHeight));
    map.insert(KeyPair(keys::kWidthKey, kWidth));
  }

  return map;
}

bool ParsePreferenceItem(const base::DictionaryValue* in_value,
                         base::DictionaryValue* out_value,
                         std::set<std::string>* used) {
  DCHECK(in_value && in_value->IsType(base::Value::TYPE_DICTIONARY));

  std::string pref_name;
  std::string pref_value;
  std::string pref_readonly;
  if (in_value->GetString(keys::kPreferencesNameKey, &pref_name)
     && used->find(pref_name) == used->end()) {
    out_value->SetString(kPreferencesName, pref_name);
    used->insert(pref_name);
  } else {
    return false;
  }

  if (in_value->GetString(keys::kPreferencesValueKey, &pref_value))
    out_value->SetString(kPreferencesValue, pref_value);

  if (in_value->GetString(keys::kPreferencesReadonlyKey, &pref_readonly))
    out_value->SetBoolean(kPreferencesReadonly, pref_readonly == "true");
  return true;
}

}  // namespace

namespace application {

WidgetInfo::WidgetInfo()
    : value_(new base::DictionaryValue) {}

WidgetInfo::~WidgetInfo() {}

void WidgetInfo::SetString(const std::string& key, const std::string& value) {
  value_->SetString(key, value);
}

void WidgetInfo::Set(const std::string& key, base::Value* value) {
  value_->Set(key, value);
}

void WidgetInfo::SetName(const std::string& name) {
  value_->SetString(kName, name);
}

void WidgetInfo::SetShortName(const std::string& short_name) {
  value_->SetString(kShortName, short_name);
}

void WidgetInfo::SetDescription(const std::string& description) {
  value_->SetString(kDecription, description);
}

WidgetHandler::WidgetHandler() {}

WidgetHandler::~WidgetHandler() {}

bool WidgetHandler::Parse(scoped_refptr<ApplicationData> application,
                          base::string16* error) {
  scoped_ptr<WidgetInfo> widget_info(new WidgetInfo);
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);

  const KeyMap& map = GetWidgetKeyPairs();

  for (KeyMapIterator iter = map.begin(); iter != map.end(); ++iter) {
    std::string string;
    bool result = manifest->GetString(iter->first, &string);
    if (result && !string.empty() && iter->first == keys::kAuthorHrefKey &&
        !GURL(string).is_valid())
      // When authorhref is an invalid URI, reset it an empty string.
      string.clear();
    widget_info->SetString(iter->second, result ? string : "");
  }

  base::Value* pref_value = NULL;
  manifest->Get(keys::kPreferencesKey, &pref_value);

  std::set<std::string> preference_names_used;
  if (pref_value && pref_value->IsType(base::Value::TYPE_DICTIONARY)) {
    base::DictionaryValue* preferences = new base::DictionaryValue;
    base::DictionaryValue* dict;
    pref_value->GetAsDictionary(&dict);
    if (ParsePreferenceItem(dict, preferences, &preference_names_used))
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
      if (ParsePreferenceItem(dict, pref, &preference_names_used))
        preferences->Append(pref);
    }
    widget_info->Set(kPreferences, preferences);
  }

  application->SetManifestData(keys::kWidgetKey, widget_info.release());
  return true;
}

bool WidgetHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error) const {
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);
  std::string ns_value;
  if (!manifest->GetString(keys::kWidgetNamespaceKey, &ns_value)) {
    *error = std::string("Failed to retrieve the widget's namespace.");
    return false;
  }
  if (strcasecmp(keys::kWidgetNamespacePrefix, ns_value.c_str()) != 0) {
    *error = std::string("The widget namespace is invalid.");
    return false;
  }
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
