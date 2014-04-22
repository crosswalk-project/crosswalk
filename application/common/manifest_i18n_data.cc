// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_i18n_data.h"

#include <map>
#include <utility>

#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace {
const char kAutoLocale[] = "en-gb";
const char kDefalut[] = "default";

const std::string GetLocalKey(const std::string local,
                              const std::string key) {
  std::string local_key(local);
  if (local_key.empty())
    local_key.append(kDefalut);
  local_key.append(".");
  local_key.append(key);
  return local_key;
}
}  // namespace

namespace application {

ManifestI18NData::ManifestI18NData()
    : value_(new base::DictionaryValue) {}

void ManifestI18NData::ParseFromPath(const Manifest* manifest,
                                     const std::string& path) {
  base::Value* value = NULL;
  manifest->Get(path, &value);
  if (value) {
    if (value->IsType(base::Value::TYPE_DICTIONARY)) {
      base::DictionaryValue* dict;
      value->GetAsDictionary(&dict);
      ParseEachElement(dict, path);
    } else if (value->IsType(base::Value::TYPE_LIST)) {
      base::ListValue* list;
      value->GetAsList(&list);
      for (base::ListValue::iterator it = list->begin();
         it != list->end(); ++it) {
        base::DictionaryValue* dict;
        (*it)->GetAsDictionary(&dict);
        ParseEachElement(dict, path);
      }
    }
  }
}

bool ManifestI18NData::GetString(const std::string& locale,
                                 const std::string& key,
                                 std::string* out_value) const {
  return value_->GetString(GetLocalKey(locale, key), out_value);
}

bool ManifestI18NData::GetString(const std::string& key,
                                  std::string* out_value) const {
  bool get = value_->GetString(GetLocalKey(kAutoLocale, key), out_value);
  get = get || value_->GetString(GetLocalKey(kDefalut, key), out_value);
  get = get || value_->GetString(GetLocalKey(default_locale_, key), out_value);
  // TODO(Hongzhang): Get system locale like system_locale = GetSystemLocale();
  std::string system_locale;
  return get || value_->GetString(GetLocalKey(system_locale, key), out_value);
}

void ManifestI18NData::ParseEachElement(const base::DictionaryValue* dict,
                                        const std::string& path) {
  std::string xml_lang;
  dict->GetString(keys::kXmlLangKey, &xml_lang);

  base::DictionaryValue::Iterator iter(*dict);
  while (!iter.IsAtEnd()) {
    if (iter.value().IsType(base::Value::TYPE_STRING)) {
      std::string string_value;
      iter.value().GetAsString(&string_value);
      std::string keyWithPath(path);
      keyWithPath.append(".");
      keyWithPath.append(iter.key());
      value_->SetString(GetLocalKey(xml_lang, keyWithPath), string_value);
    }
    iter.Advance();
  }
}

}  // namespace application
}  // namespace xwalk
