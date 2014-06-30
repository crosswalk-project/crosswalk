// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest.h"

#include <list>

#include "base/basictypes.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/runtime/common/xwalk_system_locale.h"

namespace errors = xwalk::application_manifest_errors;
namespace keys   = xwalk::application_manifest_keys;
namespace widget_keys = xwalk::application_widget_keys;

namespace xwalk {
namespace application {
namespace {
const char kLocaleUnlocalized[] = "@unlocalized";
#if defined(OS_TIZEN)
const char kLocaleAuto[] = "en-gb";
#else
const char kLocaleAuto[] = "en-us";
#endif
const char kLocaleFirstOne[] = "*";

const char kWidgetNamePath[] = "widget.name";
const char kWidgetDecriptionPath[] = "widget.description";
const char kWidgetLicensePath[] = "widget.license";

const char kPathConnectSymbol = '.';

typedef std::list<std::string> List;

std::string GetLocalizedKey(const std::string& key,
                            const std::string& local) {
  std::string lower_local = StringToLowerASCII(local);
  if (lower_local.empty())
    lower_local = kLocaleUnlocalized;
  return key + kPathConnectSymbol + lower_local;
}

scoped_ptr<List> ExpandUserAgentLocalesList(const scoped_ptr<List>& list) {
  scoped_ptr<List> expansion_list(new List);
  for (List::const_iterator it = list->begin(); it != list->end(); ++it) {
    std::string copy_locale(*it);
    size_t position;
    do {
      expansion_list->push_back(copy_locale);
      position = copy_locale.find_last_of("-");
      copy_locale = copy_locale.substr(0, position);
    } while (position != std::string::npos);
  }
  return expansion_list.Pass();
}

}  // namespace

Manifest::Manifest(SourceType source_type,
        scoped_ptr<base::DictionaryValue> value)
    : source_type_(source_type),
      data_(value.Pass()),
      i18n_data_(new base::DictionaryValue),
      type_(TYPE_UNKNOWN) {
  if (data_->Get(keys::kStartURLKey, NULL)) {
    type_ = TYPE_PACKAGED_APP;
  } else if (data_->HasKey(keys::kAppKey)) {
    if (data_->Get(keys::kWebURLsKey, NULL) ||
        data_->Get(keys::kLaunchWebURLKey, NULL)) {
      type_ = TYPE_HOSTED_APP;
    } else if (data_->Get(keys::kLaunchLocalPathKey, NULL)) {
      type_ = TYPE_PACKAGED_APP;
    }
  }

  if (data_->HasKey(widget_keys::kWidgetKey) &&
      data_->Get(widget_keys::kWidgetKey, NULL))
    ParseWGTI18n();

  SetSystemLocale(GetSystemLocale());
}

Manifest::~Manifest() {
}

bool Manifest::ValidateManifest(
    std::string* error,
    std::vector<InstallWarning>* warnings) const {
  // TODO(changbin): field 'manifest_version' of manifest.json is not clearly
  // defined at present. Temporarily disable check of this field.
  /*
  *error = "";
  if (type_ == Manifest::TYPE_PACKAGED_APP && GetManifestVersion() < 2) {
    *error = errors::kPlatformAppNeedsManifestVersion2;
    return false;
  }
  */

  // TODO(xiang): support features validation
  return true;
}

bool Manifest::HasKey(const std::string& key) const {
  return CanAccessKey(key) && data_->HasKey(key);
}

bool Manifest::HasPath(const std::string& path) const {
  base::Value* ignored = NULL;
  return CanAccessPath(path) && data_->Get(path, &ignored);
}

bool Manifest::Get(
    const std::string& path, const base::Value** out_value) const {
  return CanAccessPath(path) && data_->Get(path, out_value);
}

bool Manifest::Get(
    const std::string& path, base::Value** out_value) const {
  return this->Get(
      path,
      const_cast<const base::Value**>(out_value));
}

bool Manifest::GetBoolean(
    const std::string& path, bool* out_value) const {
  return CanAccessPath(path) && data_->GetBoolean(path, out_value);
}

bool Manifest::GetInteger(
    const std::string& path, int* out_value) const {
  return CanAccessPath(path) && data_->GetInteger(path, out_value);
}

bool Manifest::GetString(
    const std::string& path, std::string* out_value) const {
  if (!CanAccessPath(path))
    return false;

  if (i18n_data_->Get(path, NULL)) {
    List::const_iterator it = user_agent_locales_->begin();
    for (; it != user_agent_locales_->end(); ++it) {
      if (i18n_data_->GetString(GetLocalizedKey(path, *it), out_value))
        return true;
    }
    return false;
  }

  return data_->GetString(path, out_value);
}

bool Manifest::GetString(
    const std::string& path, base::string16* out_value) const {
  if (!CanAccessPath(path))
    return false;

  if (i18n_data_->Get(path, NULL)) {
    List::const_iterator it = user_agent_locales_->begin();
    for (; it != user_agent_locales_->end(); ++it) {
      if (i18n_data_->GetString(GetLocalizedKey(path, *it), out_value))
        return true;
    }
    return false;
  }

  return data_->GetString(path, out_value);
}

bool Manifest::GetDictionary(
    const std::string& path, const base::DictionaryValue** out_value) const {
  return CanAccessPath(path) && data_->GetDictionary(path, out_value);
}

bool Manifest::GetList(
    const std::string& path, const base::ListValue** out_value) const {
  return CanAccessPath(path) && data_->GetList(path, out_value);
}

Manifest* Manifest::DeepCopy() const {
  Manifest* manifest = new Manifest(
      source_type_, scoped_ptr<base::DictionaryValue>(data_->DeepCopy()));
  manifest->SetApplicationID(application_id_);
  return manifest;
}

bool Manifest::Equals(const Manifest* other) const {
  return other && data_->Equals(other->value());
}

int Manifest::GetManifestVersion() const {
  int manifest_version = 1;
  data_->GetInteger(keys::kManifestVersionKey, &manifest_version);
  return manifest_version;
}

bool Manifest::CanAccessPath(const std::string& path) const {
  return true;
}

bool Manifest::CanAccessKey(const std::string& key) const {
  return true;
}

void Manifest::SetSystemLocale(const std::string& locale) {
  scoped_ptr<List> list_for_expand(new List);
  list_for_expand->push_back(locale);
  list_for_expand->push_back(default_locale_);
  list_for_expand->push_back(kLocaleUnlocalized);
  list_for_expand->push_back(kLocaleAuto);
  list_for_expand->push_back(kLocaleFirstOne);
  user_agent_locales_ = ExpandUserAgentLocalesList(list_for_expand);
}

void Manifest::ParseWGTI18n() {
  data_->GetString(application_widget_keys::kDefaultLocaleKey,
                   &default_locale_);
  default_locale_ = StringToLowerASCII(default_locale_);

  ParseWGTI18nEachPath(kWidgetNamePath);
  ParseWGTI18nEachPath(kWidgetDecriptionPath);
  ParseWGTI18nEachPath(kWidgetLicensePath);
}

// We might get one element of a list of element from path,
// and we parse each element for fast access.
// For example config.xml is:
// <widget>
//   <name>unlocalized name</name>
//   <name xml:lang="zh-CN">zh-CN name</name>
//   <name xml:lang="en-US" short="en-US short">en-US name</name>
// </widget>
// The path for value in i18n_data_ are :
// "widget.name.#text.@unlocalized" => "unlocalized name".
// "widget.name.#text.zh-cn" => "zh-CN name".
// "widget.name.#text.en-us" => "en-US name".
// "widget.name.@short.en-us" => "en-US short".
// "widget.name.#text.*" => "unlocalized name". (the first one)
// "widget.name.@short.*" => "". (the first one do not have a short name)
void Manifest::ParseWGTI18nEachPath(const std::string& path) {
  base::Value* value = NULL;
  if (!data_->Get(path, &value))
    return;

  if (value->IsType(base::Value::TYPE_DICTIONARY)) {
    ParseWGTI18nEachElement(value, path);
    ParseWGTI18nEachElement(value, path, kLocaleFirstOne);
  } else if (value->IsType(base::Value::TYPE_LIST)) {
    base::ListValue* list;
    value->GetAsList(&list);

    bool get_first_one = false;
    for (base::ListValue::iterator it = list->begin();
        it != list->end(); ++it) {
      ParseWGTI18nEachElement(*it, path);
      if (!get_first_one)
        get_first_one = ParseWGTI18nEachElement(*it, path, kLocaleFirstOne);
    }
  }
}

bool Manifest::ParseWGTI18nEachElement(base::Value* value,
                                       const std::string& path,
                                       const std::string& locale) {
  base::DictionaryValue* dict;
  if (!value->GetAsDictionary(&dict))
    return false;

  std::string xml_lang(locale);
  if (locale.empty())
    dict->GetString(application_widget_keys::kXmlLangKey, &xml_lang);

  base::DictionaryValue::Iterator iter(*dict);
  while (!iter.IsAtEnd()) {
    std::string locale_key(
        GetLocalizedKey(path + kPathConnectSymbol + iter.key(), xml_lang));
    if (!i18n_data_->Get(locale_key, NULL))
      i18n_data_->Set(locale_key, iter.value().DeepCopy());

    iter.Advance();
  }

  return true;
}

}  // namespace application
}  // namespace xwalk
