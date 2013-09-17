// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest.h"

#include "base/basictypes.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace errors = xwalk::application_manifest_errors;
namespace keys   = xwalk::application_manifest_keys;

namespace xwalk {
namespace application {

Manifest::Manifest(SourceType source_type,
        scoped_ptr<base::DictionaryValue> value)
    : source_type_(source_type),
      data_(value.Pass()),
      type_(TYPE_UNKNOWN) {
  if (data_->HasKey(keys::kAppKey)) {
    if (data_->Get(keys::kWebURLsKey, NULL) ||
        data_->Get(keys::kLaunchWebURLKey, NULL)) {
      type_ = TYPE_HOSTED_APP;
    } else if (data_->Get(keys::kAppMainKey, NULL) ||
               data_->Get(keys::kLaunchLocalPathKey, NULL)) {
      type_ = TYPE_PACKAGED_APP;
    }
  }
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
  return CanAccessPath(path) && data_->GetString(path, out_value);
}

bool Manifest::GetString(
    const std::string& path, string16* out_value) const {
  return CanAccessPath(path) && data_->GetString(path, out_value);
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

}  // namespace application
}  // namespace xwalk
