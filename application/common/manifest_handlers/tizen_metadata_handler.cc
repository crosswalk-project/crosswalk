// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_metadata_handler.h"

#include <map>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "xwalk/application/common/application_manifest_constants.h"


namespace xwalk {

namespace keys = application_widget_keys;

typedef std::pair<std::string, std::string> MetaDataPair;
typedef std::map<std::string, std::string> MetaDataMap;
typedef std::map<std::string, std::string>::const_iterator MetaDataIter;

namespace {

MetaDataPair ParseMetaDataItem(const base::DictionaryValue* dict,
                               base::string16* error) {
  DCHECK(dict && dict->IsType(base::Value::TYPE_DICTIONARY));
  MetaDataPair result;
  std::string value;
  if (!dict->GetString(keys::kTizenMetaDataNameKey, &result.first) ||
      !dict->GetString(keys::kTizenMetaDataValueKey, &result.second)) {
    *error = base::ASCIIToUTF16("Invalid key/value of tizen metaData.");
  }

  return result;
}
}  // namespace

namespace application {

TizenMetaDataInfo::TizenMetaDataInfo() {}

TizenMetaDataInfo::~TizenMetaDataInfo() {}

bool TizenMetaDataInfo::HasKey(const std::string& key) const {
  return metadata_.find(key) != metadata_.end();
}

std::string TizenMetaDataInfo::GetValue(const std::string& key) const {
  MetaDataIter it = metadata_.find(key);
  if (it != metadata_.end())
    return it->second;
  return std::string("");
}

void TizenMetaDataInfo::SetValue(const std::string& key,
                                 const std::string& value) {
  metadata_.insert(MetaDataPair(key, value));
}

TizenMetaDataHandler::TizenMetaDataHandler() {
}

TizenMetaDataHandler::~TizenMetaDataHandler() {}

bool TizenMetaDataHandler::Parse(scoped_refptr<ApplicationData> application,
                                 base::string16* error) {
  scoped_ptr<TizenMetaDataInfo> metadata_info(new TizenMetaDataInfo);
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);

  base::Value* metadata_value = NULL;
  if (!manifest->Get(keys::kTizenMetaDataKey, &metadata_value)) {
    *error = base::ASCIIToUTF16("Failed to get value of tizen metaData");
  }

  MetaDataPair metadata_item;
  if (metadata_value && metadata_value->IsType(base::Value::TYPE_DICTIONARY)) {
    base::DictionaryValue* dict;
    metadata_value->GetAsDictionary(&dict);
    metadata_item = ParseMetaDataItem(dict, error);
    metadata_info->SetValue(metadata_item.first, metadata_item.second);
  } else if (metadata_value && metadata_value->IsType(base::Value::TYPE_LIST)) {
    base::ListValue* list;
    metadata_value->GetAsList(&list);

    for (base::ListValue::iterator it = list->begin();
         it != list->end(); ++it) {
      base::DictionaryValue* dict;
      (*it)->GetAsDictionary(&dict);
      metadata_item = ParseMetaDataItem(dict, error);
      metadata_info->SetValue(metadata_item.first, metadata_item.second);
    }
  }

  application->SetManifestData(keys::kTizenMetaDataKey,
                               metadata_info.release());
  return true;
}

std::vector<std::string> TizenMetaDataHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenMetaDataKey);
}

}  // namespace application
}  // namespace xwalk
