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
    Manifest::SourceType source_type,
    const base::DictionaryValue& manifest_data,
    const std::string& explicit_id,
    std::string* error_message) {
  DCHECK(error_message);
  base::string16 error;
  scoped_ptr<xwalk::application::Manifest> manifest(
      new xwalk::application::Manifest(source_type,
                 scoped_ptr<base::DictionaryValue>(manifest_data.DeepCopy())));

  if (!InitApplicationID(manifest.get(), path, explicit_id, &error)) {
    *error_message = base::UTF16ToUTF8(error);
    return NULL;
  }

  std::vector<InstallWarning> install_warnings;
  if (!manifest->ValidateManifest(error_message, &install_warnings)) {
    return NULL;
  }

  scoped_refptr<ApplicationData> application = new ApplicationData(path,
                                                           manifest.Pass());
  application->install_warnings_.swap(install_warnings);

  if (!application->Init(&error)) {
    *error_message = base::UTF16ToUTF8(error);
    return NULL;
  }

  return application;
}

// static
bool ApplicationData::IsIDValid(const std::string& id) {
  std::string temp = StringToLowerASCII(id);

#if defined(OS_TIZEN)
  // An ID with 10 characters is most likely a legacy Tizen ID.
  if (temp.size() == kLegacyTizenIdSize) {
    for (size_t i = 0; i < kLegacyTizenIdSize; ++i) {
      const char c = temp[i];
      const bool valid = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z');
      if (!valid)
        return false;
    }

    return true;
  }
#endif

  // Verify that the id is legal.
  if (temp.size() != (kIdSize * 2))
    return false;

  // We only support lowercase IDs, because IDs can be used as URL components
  // (where GURL will lowercase it).
  for (size_t i = 0; i < temp.size(); ++i)
    if (temp[i] < 'a' || temp[i] > 'p')
      return false;

  return true;
}

// static
GURL ApplicationData::GetBaseURLFromApplicationId(
    const std::string& application_id) {
  return GURL(std::string(xwalk::application::kApplicationScheme) +
              content::kStandardSchemeSeparator + application_id + "/");
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

Manifest::SourceType ApplicationData::GetSourceType() const {
  return manifest_->GetSourceType();
}

const std::string& ApplicationData::ID() const {
  return manifest_->GetApplicationID();
}

const std::string ApplicationData::VersionString() const {
  if (!version_->components().empty())
    return Version()->GetString();

  return "";
}

bool ApplicationData::IsPlatformApp() const {
  return manifest_->IsPackaged();
}

bool ApplicationData::IsHostedApp() const {
  return GetManifest()->IsHosted();
}

// static
bool ApplicationData::InitApplicationID(xwalk::application::Manifest* manifest,
                                const base::FilePath& path,
                                const std::string& explicit_id,
                                base::string16* error) {
  std::string application_id;
#if defined(OS_TIZEN)
  if (manifest->HasKey(keys::kTizenAppIdKey)) {
    if (!manifest->GetString(keys::kTizenAppIdKey, &application_id)) {
      NOTREACHED() << "Could not get Tizen application key";
      return false;
    }
  }

  if (!application_id.empty()) {
    manifest->SetApplicationID(application_id);
    return true;
  }
#endif

  if (!explicit_id.empty()) {
    manifest->SetApplicationID(explicit_id);
    return true;
  }

  application_id = GenerateIdForPath(path);
  if (application_id.empty()) {
    NOTREACHED() << "Could not create ID from path.";
    return false;
  }
  manifest->SetApplicationID(application_id);
  return true;
}

ApplicationData::ApplicationData(const base::FilePath& path,
                     scoped_ptr<xwalk::application::Manifest> manifest)
    : manifest_version_(0),
      is_dirty_(false),
      manifest_(manifest.release()),
      finished_parsing_manifest_(false) {
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
  DCHECK(application_url.SchemeIs(xwalk::application::kApplicationScheme));
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

bool ApplicationData::Init(base::string16* error) {
  DCHECK(error);

  if (!LoadName(error))
    return false;
  if (!LoadVersion(error))
      return false;
  if (!LoadDescription(error))
      return false;
  if (!LoadManifestVersion(error))
    return false;

  application_url_ = ApplicationData::GetBaseURLFromApplicationId(ID());

  ManifestHandlerRegistry* registry =
      ManifestHandlerRegistry::GetInstance(GetPackageType());
  if (!registry->ParseAppManifest(this, error))
    return false;

  finished_parsing_manifest_ = true;
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
  std::string version_key(GetVersionKey(GetPackageType()));

  if (!manifest_->GetString(version_key, &version_str) &&
      package_type_ == Package::XPK) {
    *error = base::ASCIIToUTF16(errors::kInvalidVersion);
    return false;
  }
  version_.reset(new base::Version(version_str));
  if (package_type_ == Package::XPK &&
      (!version_->IsValid() || version_->components().size() > 4)) {
    *error = base::ASCIIToUTF16(errors::kInvalidVersion);
    return false;
  }
  return true;
}

bool ApplicationData::LoadDescription(base::string16* error) {
  DCHECK(error);
  if (manifest_->HasKey(keys::kDescriptionKey) &&
      !manifest_->GetString(keys::kDescriptionKey, &description_) &&
      package_type_ == Package::XPK) {
    *error = base::ASCIIToUTF16(errors::kInvalidDescription);
    return false;
  }
  return true;
}

bool ApplicationData::LoadManifestVersion(base::string16* error) {
  DCHECK(error);
  // Get the original value out of the dictionary so that we can validate it
  // more strictly.
  if (manifest_->value()->HasKey(keys::kManifestVersionKey)) {
    int manifest_version = 1;
    if (!manifest_->GetInteger(keys::kManifestVersionKey, &manifest_version) ||
        manifest_version < 1) {
      if (package_type_ == Package::XPK) {
        *error = base::ASCIIToUTF16(errors::kInvalidManifestVersion);
        return false;
      }
    }
  }

  manifest_version_ = manifest_->GetManifestVersion();
  return true;
}

StoredPermission ApplicationData::GetPermission(
    std::string& permission_name) const {
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
    is_dirty_ = true;
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

#if defined(OS_TIZEN)
bool ApplicationData::HasCSPDefined() const {
  return (manifest_->HasPath(widget_keys::kCSPKey) ||
          manifest_->HasPath(widget_keys::kCSPReportOnlyKey) ||
          manifest_->HasPath(widget_keys::kAllowNavigationKey));
}
#endif

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
