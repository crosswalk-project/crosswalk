// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application.h"

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
#include "content/public/common/url_constants.h"
#include "url/url_util.h"
#include "ui/base/l10n/l10n_util.h"

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/tizen/appcore_context.h"
#endif

namespace keys = xwalk::application_manifest_keys;
namespace errors = xwalk::application_manifest_errors;

namespace xwalk {
namespace application {

// static
scoped_refptr<Application> Application::Create(const base::FilePath& path,
                                           Manifest::SourceType source_type,
                                           const DictionaryValue& manifest_data,
                                           const std::string& explicit_id,
                                           std::string* error_message) {
  DCHECK(error_message);
  string16 error;
  scoped_ptr<xwalk::application::Manifest> manifest(
      new xwalk::application::Manifest(source_type,
                 scoped_ptr<DictionaryValue>(manifest_data.DeepCopy())));

  if (!InitApplicationID(manifest.get(), path, explicit_id, &error)) {
    *error_message = UTF16ToUTF8(error);
    return NULL;
  }

  std::vector<InstallWarning> install_warnings;
  if (!manifest->ValidateManifest(error_message, &install_warnings)) {
    return NULL;
  }

  scoped_refptr<Application> application = new Application(path,
                                                           manifest.Pass());
  application->install_warnings_.swap(install_warnings);

  if (!application->Init(&error)) {
    *error_message = UTF16ToUTF8(error);
    return NULL;
  }

  return application;
}

// static
bool Application::IsIDValid(const std::string& id) {
  // Verify that the id is legal.
  if (id.size() != (kIdSize * 2))
    return false;

  // We only support lowercase IDs, because IDs can be used as URL components
  // (where GURL will lowercase it).
  std::string temp = StringToLowerASCII(id);
  for (size_t i = 0; i < temp.size(); ++i)
    if (temp[i] < 'a' || temp[i] > 'p')
      return false;

  return true;
}

// static
GURL Application::GetBaseURLFromApplicationId(
    const std::string& application_id) {
  return GURL(std::string(xwalk::application::kApplicationScheme) +
              content::kStandardSchemeSeparator + application_id + "/");
}

Application::ManifestData* Application::GetManifestData(const std::string& key)
    const {
  DCHECK(finished_parsing_manifest_ || thread_checker_.CalledOnValidThread());
  ManifestDataMap::const_iterator iter = manifest_data_.find(key);
  if (iter != manifest_data_.end())
    return iter->second.get();
  return NULL;
}

void Application::SetManifestData(const std::string& key,
                                Application::ManifestData* data) {
  DCHECK(!finished_parsing_manifest_ && thread_checker_.CalledOnValidThread());
  manifest_data_[key] = linked_ptr<ManifestData>(data);
}

Manifest::SourceType Application::GetSourceType() const {
  return manifest_->GetSourceType();
}

const std::string& Application::ID() const {
  return manifest_->GetApplicationID();
}

const std::string Application::VersionString() const {
  return Version()->GetString();
}

bool Application::IsPlatformApp() const {
  return manifest_->IsPackaged();
}

bool Application::IsHostedApp() const {
  return GetManifest()->IsHosted();
}

// static
bool Application::InitApplicationID(xwalk::application::Manifest* manifest,
                                const base::FilePath& path,
                                const std::string& explicit_id,
                                string16* error) {
  if (!explicit_id.empty()) {
    manifest->SetApplicationID(explicit_id);
    return true;
  }

  std::string application_id = GenerateIdForPath(path);
  if (application_id.empty()) {
    NOTREACHED() << "Could not create ID from path.";
    return false;
  }
  manifest->SetApplicationID(application_id);
  return true;
}

Application::Application(const base::FilePath& path,
                     scoped_ptr<xwalk::application::Manifest> manifest)
    : manifest_version_(0),
      manifest_(manifest.release()),
      finished_parsing_manifest_(false) {
  DCHECK(path.empty() || path.IsAbsolute());
  path_ = path;
}

Application::~Application() {
}

// static
GURL Application::GetResourceURL(const GURL& application_url,
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

Manifest::Type Application::GetType() const {
  return manifest_->GetType();
}

bool Application::Init(string16* error) {
  DCHECK(error);

  if (!LoadName(error))
    return false;
  if (!LoadVersion(error))
      return false;
  if (!LoadDescription(error))
      return false;
  if (!LoadManifestVersion(error))
    return false;

  application_url_ = Application::GetBaseURLFromApplicationId(ID());

  if (!ManifestHandlerRegistry::GetInstance()->ParseAppManifest(this, error))
    return false;

  finished_parsing_manifest_ = true;
#if defined(OS_TIZEN_MOBILE)
  appcore_context_ = tizen::AppcoreContext::Create();
#endif
  return true;
}

bool Application::LoadName(string16* error) {
  DCHECK(error);
  string16 localized_name;
  if (!manifest_->GetString(keys::kNameKey, &localized_name)) {
    *error = ASCIIToUTF16(errors::kInvalidName);
    return false;
  }
  non_localized_name_ = UTF16ToUTF8(localized_name);
  base::i18n::AdjustStringForLocaleDirection(&localized_name);
  name_ = UTF16ToUTF8(localized_name);
  return true;
}

bool Application::LoadVersion(string16* error) {
  DCHECK(error);
  std::string version_str;
  if (!manifest_->GetString(keys::kVersionKey, &version_str)) {
    *error = ASCIIToUTF16(errors::kInvalidVersion);
    return false;
  }
  version_.reset(new base::Version(version_str));
  if (!version_->IsValid() || version_->components().size() > 4) {
    *error = ASCIIToUTF16(errors::kInvalidVersion);
    return false;
  }
  return true;
}

bool Application::LoadDescription(string16* error) {
  DCHECK(error);
  if (manifest_->HasKey(keys::kDescriptionKey) &&
      !manifest_->GetString(keys::kDescriptionKey, &description_)) {
    *error = ASCIIToUTF16(errors::kInvalidDescription);
    return false;
  }
  return true;
}

bool Application::LoadManifestVersion(string16* error) {
  DCHECK(error);
  // Get the original value out of the dictionary so that we can validate it
  // more strictly.
  if (manifest_->value()->HasKey(keys::kManifestVersionKey)) {
    int manifest_version = 1;
    if (!manifest_->GetInteger(keys::kManifestVersionKey, &manifest_version) ||
        manifest_version < 1) {
      *error = ASCIIToUTF16(errors::kInvalidManifestVersion);
      return false;
    }
  }

  manifest_version_ = manifest_->GetManifestVersion();
  return true;
}

}   // namespace application
}   // namespace xwalk
