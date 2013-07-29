// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest.h"

#include "base/basictypes.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/stringprintf.h"
#include "base/strings/string_split.h"
#include "base/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace errors = application_manifest_errors;
namespace keys = application_manifest_keys;

namespace xwalk_application {

Manifest::Manifest(Location location, scoped_ptr<base::DictionaryValue> value)
    : location_(location),
      value_(value.Pass()),
      type_(TYPE_UNKNOWN) {
  if (value_->HasKey(keys::kApp)) {
    if (value_->Get(keys::kWebURLs, NULL) ||
        value_->Get(keys::kLaunchWebURL, NULL)) {
      type_ = TYPE_HOSTED_APP;
    } else if (value_->Get(keys::kPlatformAppBackground, NULL) ||
               value_->Get(keys::kLaunchLocalPath, NULL)) {
      type_ = TYPE_PLATFORM_APP;
    }
  }
}

Manifest::~Manifest() {
}

bool Manifest::ValidateManifest(
    std::string* error,
    std::vector<InstallWarning>* warnings) const {
  *error = "";
  if (type_ == Manifest::TYPE_PLATFORM_APP && GetManifestVersion() < 2) {
    *error = errors::kPlatformAppNeedsManifestVersion2;
    return false;
  }

  // TODO(xiang): support features validation
  return true;
}

bool Manifest::HasKey(const std::string& key) const {
  return CanAccessKey(key) && value_->HasKey(key);
}

bool Manifest::HasPath(const std::string& path) const {
  base::Value* ignored = NULL;
  return CanAccessPath(path) && value_->Get(path, &ignored);
}

bool Manifest::Get(
    const std::string& path, const base::Value** out_value) const {
  return CanAccessPath(path) && value_->Get(path, out_value);
}

bool Manifest::GetBoolean(
    const std::string& path, bool* out_value) const {
  return CanAccessPath(path) && value_->GetBoolean(path, out_value);
}

bool Manifest::GetInteger(
    const std::string& path, int* out_value) const {
  return CanAccessPath(path) && value_->GetInteger(path, out_value);
}

bool Manifest::GetString(
    const std::string& path, std::string* out_value) const {
  return CanAccessPath(path) && value_->GetString(path, out_value);
}

bool Manifest::GetString(
    const std::string& path, string16* out_value) const {
  return CanAccessPath(path) && value_->GetString(path, out_value);
}

bool Manifest::GetDictionary(
    const std::string& path, const base::DictionaryValue** out_value) const {
  return CanAccessPath(path) && value_->GetDictionary(path, out_value);
}

bool Manifest::GetList(
    const std::string& path, const base::ListValue** out_value) const {
  return CanAccessPath(path) && value_->GetList(path, out_value);
}

Manifest* Manifest::DeepCopy() const {
  Manifest* manifest = new Manifest(
      location_, scoped_ptr<base::DictionaryValue>(value_->DeepCopy()));
  manifest->set_application_id(application_id_);
  return manifest;
}

bool Manifest::Equals(const Manifest* other) const {
  return other && value_->Equals(other->value());
}

int Manifest::GetManifestVersion() const {
  int manifest_version = 1;
  value_->GetInteger(keys::kManifestVersion, &manifest_version);
  return manifest_version;
}

bool Manifest::CanAccessPath(const std::string& path) const {
  return true;
}

bool Manifest::CanAccessKey(const std::string& key) const {
  return true;
}

}  // namespace xwalk_application
