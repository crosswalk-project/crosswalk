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
#include "base/string16.h"
#include "base/string_util.h"
#include "base/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "base/version.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest.h"
#include "content/public/common/url_constants.h"
#include "googleurl/src/url_util.h"
#include "ui/base/l10n/l10n_util.h"

namespace keys = application_manifest_keys;
namespace errors = application_manifest_errors;

namespace xwalk_application {

// static
scoped_refptr<Application> Application::Create(const base::FilePath& path,
                                           Manifest::Location location,
                                           const DictionaryValue& value,
                                           int flags,
                                           std::string* utf8_error) {
  return Application::Create(path,
                           location,
                           value,
                           flags,
                           std::string(),  // ID is ignored if empty.
                           utf8_error);
}

// TODO(sungguk): Continue removing std::string errors and replacing
// with string16. See http://crbug.com/71980.
scoped_refptr<Application> Application::Create(const base::FilePath& path,
                                           Manifest::Location location,
                                           const DictionaryValue& value,
                                           int flags,
                                           const std::string& explicit_id,
                                           std::string* utf8_error) {
  DCHECK(utf8_error);
  string16 error;
  scoped_ptr<xwalk_application::Manifest> manifest(
      new xwalk_application::Manifest(location,
                               scoped_ptr<DictionaryValue>(value.DeepCopy())));

  if (!InitApplicationID(manifest.get(), path, explicit_id, flags, &error)) {
    *utf8_error = UTF16ToUTF8(error);
    return NULL;
  }

  std::vector<InstallWarning> install_warnings;
  if (!manifest->ValidateManifest(utf8_error, &install_warnings)) {
    return NULL;
  }

  scoped_refptr<Application> application = new Application(path,
                                                           manifest.Pass());
  application->install_warnings_.swap(install_warnings);

  if (!application->InitFromValue(flags, &error)) {
    *utf8_error = UTF16ToUTF8(error);
    return NULL;
  }

  return application;
}

// static
bool Application::IdIsValid(const std::string& id) {
  // Verify that the id is legal.
  if (id.size() != (id_util::kIdSize * 2))
    return false;

  // We only support lowercase IDs, because IDs can be used as URL components
  // (where GURL will lowercase it).
  std::string temp = StringToLowerASCII(id);
  for (size_t i = 0; i < temp.size(); i++)
    if (temp[i] < 'a' || temp[i] > 'p')
      return false;

  return true;
}


// static
GURL Application::GetBaseURLFromApplicationId(
    const std::string& application_id) {
  return GURL(std::string(xwalk_application::kApplicationScheme) +
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

Manifest::Location Application::location() const {
  return manifest_->location();
}

const std::string& Application::id() const {
  return manifest_->application_id();
}

const std::string Application::VersionString() const {
  return version()->GetString();
}

bool Application::is_platform_app() const {
  return manifest_->is_platform_app();
}

bool Application::is_hosted_app() const {
  return manifest()->is_hosted_app();
}

// static
bool Application::InitApplicationID(xwalk_application::Manifest* manifest,
                                const base::FilePath& path,
                                const std::string& explicit_id,
                                int creation_flags,
                                string16* error) {
  if (!explicit_id.empty()) {
    manifest->set_application_id(explicit_id);
    return true;
  }

  if (creation_flags & REQUIRE_KEY) {
    *error = ASCIIToUTF16(errors::kInvalidKey);
    return false;
  } else {
    // If there is a path, we generate the ID from it. This is useful for
    // development mode, because it keeps the ID stable across restarts and
    // reloading the application.
    std::string application_id = id_util::GenerateIdForPath(path);
    if (application_id.empty()) {
      NOTREACHED() << "Could not create ID from path.";
      return false;
    }
    manifest->set_application_id(application_id);
    return true;
  }
}

Application::Application(const base::FilePath& path,
                     scoped_ptr<xwalk_application::Manifest> manifest)
    : manifest_version_(0),
      manifest_(manifest.release()),
      finished_parsing_manifest_(false),
      creation_flags_(0) {
  DCHECK(path.empty() || path.IsAbsolute());
  path_ = path;
}

Application::~Application() {
}

// static
GURL Application::GetResourceURL(const GURL& application_url,
                               const std::string& relative_path) {
  DCHECK(application_url.SchemeIs(xwalk_application::kApplicationScheme));
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
  return manifest_->type();
}

bool Application::InitFromValue(int flags, string16* error) {
  DCHECK(error);

  creation_flags_ = flags;

  // Important to load manifest version first because many other features
  // depend on its value.
  if (!LoadManifestVersion(error))
    return false;

  application_url_ = Application::GetBaseURLFromApplicationId(id());

  if (!LoadSharedFeatures(error))
    return false;

  finished_parsing_manifest_ = true;

  return true;
}

bool Application::LoadName(string16* error) {
  string16 localized_name;
  if (!manifest_->GetString(keys::kName, &localized_name)) {
    *error = ASCIIToUTF16(errors::kInvalidName);
    return false;
  }
  non_localized_name_ = UTF16ToUTF8(localized_name);
  base::i18n::AdjustStringForLocaleDirection(&localized_name);
  name_ = UTF16ToUTF8(localized_name);
  return true;
}

bool Application::LoadVersion(string16* error) {
  std::string version_str;
  if (!manifest_->GetString(keys::kVersion, &version_str)) {
    *error = ASCIIToUTF16(errors::kInvalidVersion);
    return false;
  }
  version_.reset(new Version(version_str));
  if (!version_->IsValid() || version_->components().size() > 4) {
    *error = ASCIIToUTF16(errors::kInvalidVersion);
    return false;
  }
  return true;
}

bool Application::LoadDescription(string16* error) {
  if (manifest_->HasKey(keys::kDescription) &&
      !manifest_->GetString(keys::kDescription, &description_)) {
    *error = ASCIIToUTF16(errors::kInvalidDescription);
    return false;
  }
  return true;
}

bool Application::LoadManifestVersion(string16* error) {
  // Get the original value out of the dictionary so that we can validate it
  // more strictly.
  if (manifest_->value()->HasKey(keys::kManifestVersion)) {
    int manifest_version = 1;
    if (!manifest_->GetInteger(keys::kManifestVersion, &manifest_version) ||
        manifest_version < 1) {
      *error = ASCIIToUTF16(errors::kInvalidManifestVersion);
      return false;
    }
  }

  manifest_version_ = manifest_->GetManifestVersion();

  return true;
}

bool Application::LoadSharedFeatures(string16* error) {
  if (!LoadDescription(error))
    return false;

  return true;
}

}   // namespace xwalk_application
