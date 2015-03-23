// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_category_handler.h"

#include "base/memory/scoped_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {

const char kErrMsgCategory[] =
    "Parsing category element failed";
const char kErrMsgCategoryName[] =
    "The name element inside category element is obligatory";

bool ParseCategoryEntryAndStore(const base::DictionaryValue& control_dict,
    CategoryInfoList* aplist) {
  std::string name;
  if (!control_dict.GetString(keys::kTizenCategoryNameKey, &name))
    return false;
  aplist->categories.push_back(name);
  return true;
}

}  // namespace

TizenCategoryHandler::TizenCategoryHandler() {
}

TizenCategoryHandler::~TizenCategoryHandler() {
}

bool TizenCategoryHandler::Parse(scoped_refptr<ApplicationData> application,
    base::string16* error) {
  const Manifest* manifest = application->GetManifest();
  scoped_ptr<CategoryInfoList> aplist(new CategoryInfoList());
  base::Value* value = nullptr;
  if (!manifest->Get(keys::kTizenCategoryKey, &value))
    return true;

  if (value->GetType() == base::Value::TYPE_LIST) {
    // multiple entries
    const base::ListValue* list;
    value->GetAsList(&list);
    for (const auto& item : *list) {
      const base::DictionaryValue* control_dict;
      if (!item->GetAsDictionary(&control_dict) ||
          !ParseCategoryEntryAndStore(*control_dict, aplist.get())) {
        *error = base::ASCIIToUTF16(kErrMsgCategory);
        return false;
      }
    }
  } else if (value->GetType() == base::Value::TYPE_DICTIONARY) {
    // single entry
    const base::DictionaryValue* dict;
    value->GetAsDictionary(&dict);
    if (!ParseCategoryEntryAndStore(*dict, aplist.get()))
      return false;
  } else {
    *error = base::ASCIIToUTF16(kErrMsgCategory);
    return false;
  }

  application->SetManifestData(keys::kTizenCategoryKey, aplist.release());
  return true;
}

bool TizenCategoryHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error) const {
  const CategoryInfoList* categories_list =
      static_cast<const CategoryInfoList*>(
          application->GetManifestData(keys::kTizenCategoryKey));

  if (!categories_list)
    return true;

  for (const auto& item : categories_list->categories) {
    if (item.empty()) {
      *error = kErrMsgCategoryName;
      return false;
    }
  }
  return true;
}

std::vector<std::string> TizenCategoryHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenCategoryKey);
}

}  // namespace application
}  // namespace xwalk
