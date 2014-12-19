// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_app_control_handler.h"

#include "base/memory/scoped_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "base/third_party/xdg_mime/xdgmime.h"
#include "base/values.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {

void ParseAppControlEntryAndStore(const base::DictionaryValue& control_dict,
    AppControlInfoList* aplist) {
  std::string src;
  const base::DictionaryValue* src_dict;
  if (control_dict.GetDictionary(keys::kTizenApplicationAppControlSrcKey,
      &src_dict)) {
    src_dict->GetString(
        keys::kTizenApplicationAppControlChildNameAttrKey, &src);
  }

  std::string operation;
  const base::DictionaryValue* operation_dict;
  if (control_dict.GetDictionary(
      keys::kTizenApplicationAppControlOperationKey,
      &operation_dict)) {
    operation_dict->GetString(
        keys::kTizenApplicationAppControlChildNameAttrKey, &operation);
  }

  std::string uri;
  const base::DictionaryValue* uri_dict;
  if (control_dict.GetDictionary(keys::kTizenApplicationAppControlUriKey,
      &uri_dict)) {
    uri_dict->GetString(
        keys::kTizenApplicationAppControlChildNameAttrKey, &uri);
  }

  std::string mime;
  const base::DictionaryValue* mime_dict;
  if (control_dict.GetDictionary(keys::kTizenApplicationAppControlMimeKey,
      &mime_dict)) {
    mime_dict->GetString(
        keys::kTizenApplicationAppControlChildNameAttrKey, &mime);
  }

  aplist->controls.emplace_back(src, operation, uri, mime);
}

}  // namespace

TizenAppControlHandler::TizenAppControlHandler() {
}

TizenAppControlHandler::~TizenAppControlHandler() {
}

bool TizenAppControlHandler::Parse(scoped_refptr<ApplicationData> application,
    base::string16* error) {
  const Manifest* manifest = application->GetManifest();
  scoped_ptr<AppControlInfoList> aplist(new AppControlInfoList());
  base::Value* value;
  manifest->Get(keys::kTizenApplicationAppControlsKey, &value);

  if (value->GetType() == base::Value::TYPE_LIST) {
    // multiple entries
    const base::ListValue* list;
    value->GetAsList(&list);
    for (const auto& item : *list) {
      const base::DictionaryValue* control_dict;
      if (!item->GetAsDictionary(&control_dict)) {
        *error = base::ASCIIToUTF16("Parsing app-control element failed");
        return false;
      }

      ParseAppControlEntryAndStore(*control_dict, aplist.get());
    }
  } else if (value->GetType() == base::Value::TYPE_DICTIONARY) {
    // single entry
    const base::DictionaryValue* dict;
    value->GetAsDictionary(&dict);
    ParseAppControlEntryAndStore(*dict, aplist.get());
  } else {
    *error = base::ASCIIToUTF16("Cannot parsing app-control element");
    return false;
  }

  application->SetManifestData(
      keys::kTizenApplicationAppControlsKey,
      aplist.release());
  return true;
}

bool TizenAppControlHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error) const {
  const AppControlInfoList* app_controls =
      static_cast<const AppControlInfoList*>(
          application->GetManifestData(keys::kTizenApplicationAppControlsKey));

  for (const auto& item : app_controls->controls) {
    if (item.src().empty()) {
      *error = "The src child element of app-control element is obligatory";
      return false;
    }

    const std::string& operation = item.operation();
    if (operation.empty()) {
      *error =
          "The operation child element of app-control element is obligatory";
      return false;
    }
    if (GURL(operation).spec().empty()) {
      *error =
          "The operation child element of app-control element is not valid url";
      return false;
    }

    const std::string& mime = item.mime();
    if (!mime.empty() && !xdg_mime_is_valid_mime_type(mime.c_str())) {
      *error =
          "The mime child element of app-control "
          "element is not valid mime type";
      return false;
    }
  }
  return true;
}

std::vector<std::string> TizenAppControlHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenApplicationAppControlsKey);
}

}  // namespace application
}  // namespace xwalk
