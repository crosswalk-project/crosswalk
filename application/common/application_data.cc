// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_data.h"

#include "base/base64.h"
#include "base/basictypes.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/i18n/rtl.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/stl_util.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "base/version.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/manifest_handler.h"
#include "xwalk/application/common/manifest_handlers/permissions_handler.h"
#include "xwalk/application/common/manifest_handlers/widget_handler.h"
#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"
#include "xwalk/application/common/permission_policy_manager.h"
#include "content/public/common/url_constants.h"
#include "url/url_util.h"
#include "ui/base/l10n/l10n_util.h"

namespace keys = xwalk::application_manifest_keys;
namespace widget_keys = xwalk::application_widget_keys;
namespace errors = xwalk::application_manifest_errors;

namespace xwalk {
namespace application {

// static
scoped_refptr<ApplicationData> ApplicationData::Create(
    const base::FilePath& path,
    SourceType source_type,
    const base::DictionaryValue& manifest_data,
    const std::string& explicit_id,
    std::string* error_message) {
  DCHECK(error_message);
  base::string16 error;
  scoped_ptr<Manifest> manifest(new Manifest(
      scoped_ptr<base::DictionaryValue>(manifest_data.DeepCopy())));

  if (!manifest->ValidateManifest(error_message))
    return NULL;

  scoped_refptr<ApplicationData> app_data =
      new ApplicationData(path, source_type, manifest.Pass());
  if (!app_data->Init(explicit_id, &error)) {
    *error_message = base::UTF16ToUTF8(error);
    return NULL;
  }

  return app_data;
}

// static
scoped_refptr<ApplicationData> ApplicationData::Create(
    const GURL& url,
    std::string* error_message) {
  const std::string& url_spec = url.spec();
  DCHECK(!url_spec.empty());
  const std::string& app_id = GenerateId(url_spec);

  base::DictionaryValue manifest;
  // FIXME: define permissions!
  manifest.SetString(application_manifest_keys::kStartURLKey, url_spec);
  // FIXME: Why use URL as name?
  manifest.SetString(application_manifest_keys::kNameKey, url_spec);
  manifest.SetString(application_manifest_keys::kXWalkVersionKey, "0");
  scoped_refptr<ApplicationData> application_data =
      ApplicationData::Create(base::FilePath(), EXTERNAL_URL,
                              manifest, app_id, error_message);

  return application_data;
}

// static
GURL ApplicationData::GetBaseURLFromApplicationId(
    const std::string& application_id) {
  return GURL(std::string(xwalk::application::kApplicationScheme) +
              url::kStandardSchemeSeparator + application_id + "/");
}

ApplicationData::ManifestData* ApplicationData::GetManifestData(
        const std::string& key) const {
  DCHECK(finished_parsing_manifest_ || thread_checker_.CalledOnValidThread());
  ManifestDataMap::const_iterator iter = manifest_data_.find(key);
  if (iter != manifest_data_.end())
    return iter->second.get();
  return NULL;
}

void ApplicationData::SetManifestData(const std::string& key,
                                      ApplicationData::ManifestData* data) {
  DCHECK(!finished_parsing_manifest_ && thread_checker_.CalledOnValidThread());
  manifest_data_[key] = linked_ptr<ManifestData>(data);
}

const std::string& ApplicationData::ID() const {
  return manifest_->GetApplicationID();
}

#if defined(OS_TIZEN)
std::string ApplicationData::GetPackageID() const {
  return AppIdToPkgId(manifest_->GetApplicationID());
}
#endif

const std::string ApplicationData::VersionString() const {
  if (!version_->components().empty())
    return Version()->GetString();

  return "";
}

bool ApplicationData::IsHostedApp() const {
  return GetManifest()->IsHosted();
}

ApplicationData::ApplicationData(const base::FilePath& path,
    SourceType source_type, scoped_ptr<Manifest> manifest)
    : manifest_version_(0),
      manifest_(manifest.release()),
      finished_parsing_manifest_(false),
      source_type_(source_type) {
  DCHECK(path.empty() || path.IsAbsolute());
  path_ = path;
  if (manifest_->HasPath(widget_keys::kWidgetKey))
    package_type_ = Package::WGT;
  else
    package_type_ = Package::XPK;
}

ApplicationData::~ApplicationData() {
}

// static
GURL ApplicationData::GetResourceURL(const GURL& application_url,
                               const std::string& relative_path) {
  DCHECK(application_url.SchemeIs(kApplicationScheme));
  DCHECK_EQ("/", application_url.path());

  std::string path = relative_path;

  // If the relative path starts with "/", it is "absolute" relative to the
  // application base directory, but application_url is already specified to
  // refer to that base directory, so strip the leading "/" if present.
  if (relative_path.size() > 0 && relative_path[0] == '/')
    path = relative_path.substr(1);

  GURL ret_val = GURL(application_url.spec() + path);
  DCHECK(StartsWithASCII(ret_val.spec(), application_url.spec(), false));

  return ret_val;
}

Manifest::Type ApplicationData::GetType() const {
  return manifest_->GetType();
}

bool ApplicationData::Init(const std::string& explicit_id,
                           base::string16* error) {
  DCHECK(error);
  ManifestHandlerRegistry* registry =
      ManifestHandlerRegistry::GetInstance(GetPackageType());
  if (!registry->ParseAppManifest(this, error))
    return false;

  if (!LoadID(explicit_id, error))
    return false;
  if (!LoadName(error))
    return false;
  if (!LoadVersion(error))
    return false;
  if (!LoadDescription(error))
    return false;

  application_url_ = ApplicationData::GetBaseURLFromApplicationId(ID());

  finished_parsing_manifest_ = true;
  return true;
}

bool ApplicationData::LoadID(const std::string& explicit_id,
                             base::string16* error) {
  std::string application_id;
#if defined(OS_TIZEN)
  if (GetPackageType() == Package::WGT) {
    const TizenApplicationInfo* tizen_app_info =
        static_cast<TizenApplicationInfo*>(GetManifestData(
            widget_keys::kTizenApplicationKey));
    CHECK(tizen_app_info);
    application_id = tizen_app_info->id();
  } else if (manifest_->HasKey(keys::kTizenAppIdKey)) {
    if (!manifest_->GetString(keys::kTizenAppIdKey, &application_id)) {
      NOTREACHED() << "Could not get Tizen application key";
      return false;
    }
  }

  if (!application_id.empty()) {
    manifest_->SetApplicationID(application_id);
    return true;
  }
#endif

  if (!explicit_id.empty()) {
    manifest_->SetApplicationID(explicit_id);
    return true;
  }

  application_id = GenerateIdForPath(path_);
  if (application_id.empty()) {
    NOTREACHED() << "Could not create ID from path.";
    return false;
  }
  manifest_->SetApplicationID(application_id);
  return true;
}

bool ApplicationData::LoadName(base::string16* error) {
  DCHECK(error);
  base::string16 localized_name;
  std::string name_key(GetNameKey(GetPackageType()));

  if (!manifest_->GetString(name_key, &localized_name) &&
      package_type_ == Package::XPK) {
    *error = base::ASCIIToUTF16(errors::kInvalidName);
    return false;
  }
  non_localized_name_ = base::UTF16ToUTF8(localized_name);
  base::i18n::AdjustStringForLocaleDirection(&localized_name);
  name_ = base::UTF16ToUTF8(localized_name);
  return true;
}

bool ApplicationData::LoadVersion(base::string16* error) {
  DCHECK(error);
  std::string version_str;

  version_.reset(new base::Version());

  if (package_type_ == Package::WGT) {
    bool ok = manifest_->GetString(widget_keys::kVersionKey, &version_str);
    if (!ok) {
      *error = base::ASCIIToUTF16(errors::kInvalidVersion);
      return true;
    }

    version_.reset(new base::Version(version_str));
    return true;
  }

  // W3C Manifest (XPK and hosted):

  bool hasDeprecatedKey = manifest_->HasKey(keys::kDeprecatedVersionKey);
  bool hasKey = manifest_->HasKey(keys::kXWalkVersionKey);

  if (!hasKey && !hasDeprecatedKey) {
    // xwalk_version is optional.
    return true;
  }

  bool ok = false;
  if (hasKey) {
    if (hasDeprecatedKey) {
      LOG(WARNING) << "Deprecated key '" << keys::kDeprecatedVersionKey
          << "' found in addition to '" << keys::kXWalkVersionKey
          << "'. Consider removing.";
    }
    ok = manifest_->GetString(keys::kXWalkVersionKey, &version_str);
  }

  if (!hasKey && hasDeprecatedKey) {
    LOG(WARNING) << "Deprecated key '" << keys::kDeprecatedVersionKey
        << "' found. Please migrate to using '" << keys::kXWalkVersionKey
        << "' instead.";
    ok = manifest_->GetString(keys::kDeprecatedVersionKey, &version_str);
  }

  version_.reset(new base::Version(version_str));

  if (!ok || !version_->IsValid() || version_->components().size() > 4) {
    *error = base::ASCIIToUTF16(errors::kInvalidVersion);
    version_.reset(new base::Version());
    return false;
  }

  return ok;
}

bool ApplicationData::LoadDescription(base::string16* error) {
  DCHECK(error);
  // FIXME: Better to assert on use from Widget.
  if (package_type_ != Package::XPK)
    return true;  // No error.

  bool hasDeprecatedKey = manifest_->HasKey(keys::kDeprecatedDescriptionKey);
  bool hasKey = manifest_->HasKey(keys::kXWalkDescriptionKey);

  if (hasKey) {
    if (hasDeprecatedKey) {
      LOG(WARNING) << "Deprecated key '" << keys::kDeprecatedDescriptionKey
          << "' found in addition to '" << keys::kXWalkDescriptionKey
          << "'. Consider removing.";
    }
    bool ok = manifest_->GetString(keys::kXWalkDescriptionKey, &description_);
    if (!ok)
      *error = base::ASCIIToUTF16(errors::kInvalidDescription);
    return ok;
  }

  if (hasDeprecatedKey) {
    LOG(WARNING) << "Deprecated key '" << keys::kDeprecatedDescriptionKey
        << "' found. Please migrate to using '" << keys::kXWalkDescriptionKey
        << "' instead.";
    bool ok = manifest_->GetString(
        keys::kDeprecatedDescriptionKey, &description_);
    if (!ok)
      *error = base::ASCIIToUTF16(errors::kInvalidDescription);
    return ok;
  }

  // No error but also no description found.
  return true;
}

StoredPermission ApplicationData::GetPermission(
    const std::string& permission_name) const {
  StoredPermissionMap::const_iterator iter =
      permission_map_.find(permission_name);
  if (iter == permission_map_.end())
    return UNDEFINED_STORED_PERM;
  return iter->second;
}

bool ApplicationData::SetPermission(const std::string& permission_name,
                                    StoredPermission perm) {
  if (perm != UNDEFINED_STORED_PERM) {
    permission_map_[permission_name] = perm;
    return true;
  }
  return false;
}

void ApplicationData::ClearPermissions() {
  permission_map_.clear();
}

PermissionSet ApplicationData::GetManifestPermissions() const {
  PermissionSet permissions;
  if (manifest_->value()->HasKey(keys::kPermissionsKey)) {
    const PermissionsInfo* perm_info = static_cast<PermissionsInfo*>(
                           GetManifestData(keys::kPermissionsKey));
    permissions = perm_info->GetAPIPermissions();
  }
  return permissions;
}

bool ApplicationData::HasCSPDefined() const {
#if defined(OS_TIZEN)
  return  manifest_->HasPath(GetCSPKey(package_type_)) ||
          manifest_->HasPath(widget_keys::kCSPReportOnlyKey) ||
          manifest_->HasPath(widget_keys::kAllowNavigationKey);
#else
  return manifest_->HasPath(GetCSPKey(package_type_));
#endif
}

bool ApplicationData::SetApplicationLocale(const std::string& locale,
                                           base::string16* error) {
  DCHECK(thread_checker_.CalledOnValidThread());
  manifest_->SetSystemLocale(locale);
  if (!LoadName(error))
    return false;
  if (!LoadDescription(error))
    return false;

  // Only update when the package is wgt and we have parsed the widget handler,
  // otherwise we can not get widget_info.
  if (WidgetInfo* widget_info = static_cast<WidgetInfo*>(
          GetManifestData(widget_keys::kWidgetKey))) {
    std::string string_value;
    if (manifest_->GetString(widget_keys::kNameKey, &string_value))
      widget_info->SetName(string_value);
    if (manifest_->GetString(widget_keys::kShortNameKey, &string_value))
      widget_info->SetShortName(string_value);
    if (manifest_->GetString(widget_keys::kDescriptionKey, &string_value))
      widget_info->SetDescription(string_value);
  }
  return true;
}

}   // namespace application
}   // namespace xwalk
