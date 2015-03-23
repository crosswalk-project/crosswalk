// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_ime_handler.h"

#include "base/memory/scoped_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "third_party/re2/re2/re2.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {

const char kErrMsgLanguages[] =
    "At least and only ONE tizen:languages tag should be specified";
const char kErrMsgEmptyLanguage[] =
    "Language cannot be empty";
const char kErrMsgParsingIme[] =
    "Only one ime tag should be specified";
const char kErrMsgParsingUuid[] =
    "Only one uuid tag should be specified";
const char kErrMsgValidatingUuidEmpty[] =
    "The UUID of ime element is obligatory";
const char kErrMsgUuidFormat[] =
    "Uuid should be in proper format (8-4-4-4-12)";
const char kErrMsgNoLanguages[] =
    "At least one language of ime element should be specified";

bool GetLanguage(const base::Value* item, TizenImeInfo* ime_info,
    base::string16* error) {
  const base::DictionaryValue* language_dict;
  if (item->GetAsDictionary(&language_dict)) {
    std::string language;
    if (!language_dict->GetString(keys::kTizenImeLanguageTextKey, &language) ||
        language.empty()) {
      *error = base::ASCIIToUTF16(kErrMsgEmptyLanguage);
      return false;
    }
    ime_info->AddLanguage(language);
  }
  return true;
}

bool ParseImeEntryAndStore(const base::DictionaryValue& control_dict,
    TizenImeInfo* ime_info, base::string16* error) {

  // parsing uuid element
  const base::DictionaryValue* uuid_dict;
  std::string uuid;
  if (control_dict.GetDictionary(keys::kTizenImeUuidKey, &uuid_dict) &&
      uuid_dict->GetString(keys::kTizenImeUuidTextKey, &uuid)) {
    ime_info->set_uuid(uuid);
  } else {
    *error = base::ASCIIToUTF16(kErrMsgParsingUuid);
    return false;
  }

  const base::DictionaryValue* languages_dict;
  if (!control_dict.GetDictionary(
      keys::kTizenImeLanguagesKey, &languages_dict)) {
    *error = base::ASCIIToUTF16(kErrMsgLanguages);
    return false;
  }

  const base::Value* languages;
  if (!languages_dict->Get(keys::kTizenImeLanguageKey, &languages)) {
    *error = base::ASCIIToUTF16(kErrMsgNoLanguages);
    return false;
  }

  if (languages->GetType() == base::Value::TYPE_LIST) {
    // many languages
    const base::ListValue* list;
    languages->GetAsList(&list);
    for (const auto& item : *list) {
      if (!GetLanguage(item, ime_info, error))
        return false;
    }
  } else if (languages->GetType() == base::Value::TYPE_DICTIONARY) {
    if (!GetLanguage(languages, ime_info, error))
      return false;
  }

  return true;
}

// UUID is string of 36 characters in form 8-4-4-4-12
bool IsValidUuid(const std::string& uuid) {
  const char kUuidPattern[] =
      "^[0-9a-zA-Z]{8}([-][0-9a-zA-Z]{4}){3}[-][0-9a-zA-Z]{12}$";
  return RE2::FullMatch(uuid, kUuidPattern);
}

}  // namespace

TizenImeInfo::TizenImeInfo() {
}

TizenImeInfo::~TizenImeInfo() {
}

TizenImeHandler::TizenImeHandler() {
}

TizenImeHandler::~TizenImeHandler() {
}

void TizenImeInfo::AddLanguage(const std::string& language) {
  languages_.push_back(language);
}

bool TizenImeHandler::Parse(scoped_refptr<ApplicationData> application,
    base::string16* error) {
  scoped_ptr<TizenImeInfo> ime_info(new TizenImeInfo);
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);

  base::Value* value = nullptr;
  if (!manifest->Get(keys::kTizenImeKey, &value))
    return true;

  bool result = true;

  if (value->GetType() == base::Value::TYPE_DICTIONARY) {
    // single entry
    const base::DictionaryValue* dict;
    value->GetAsDictionary(&dict);
    result = ParseImeEntryAndStore(*dict, ime_info.get(), error);
  } else {
    *error = base::ASCIIToUTF16(kErrMsgParsingIme);
    return false;
  }

  application->SetManifestData(keys::kTizenImeKey, ime_info.release());
  return result;
}

bool TizenImeHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error) const {

  const TizenImeInfo* ime_info =
      static_cast<const TizenImeInfo*>(
          application->GetManifestData(keys::kTizenImeKey));

  if (!ime_info)
    return true;

  if (ime_info->uuid().empty()) {
    *error = kErrMsgValidatingUuidEmpty;
    return false;
  }

  if (!IsValidUuid(ime_info->uuid())) {
    *error = kErrMsgUuidFormat;
    return false;
  }

  if (ime_info->languages().empty()) {
    *error = kErrMsgNoLanguages;
    return false;
  }

  return true;
}

std::vector<std::string> TizenImeHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenImeKey);
}

}  // namespace application
}  // namespace xwalk
