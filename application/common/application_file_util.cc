// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_file_util.h"

#include <algorithm>
#include <map>
#include <vector>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/file_util.h"
#include "base/json/json_file_value_serializer.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/metrics/histogram.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"
#include "net/base/escape.h"
#include "net/base/file_stream.h"
#include "xwalk/application/common/application_config_xml_loader.h"
#include "xwalk/application/browser/installer/package.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/install_warning.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/manifest_handler.h"

namespace errors = xwalk::application_manifest_errors;
namespace keys = xwalk::application_manifest_keys;
namespace widget_keys = xwalk::application_widget_keys;

namespace {
bool GetPackageType(const base::FilePath& path,
                    xwalk::application::Package::Type* package_type,
                    std::string* error) {
  base::FilePath manifest_path;

  manifest_path = path.Append(xwalk::application::kManifestXpkFilename);
  if (base::PathExists(manifest_path)) {
    *package_type = xwalk::application::Package::XPK;
    return true;
  }

  manifest_path = path.Append(xwalk::application::kManifestWgtFilename);
  if (base::PathExists(manifest_path)) {
    *package_type = xwalk::application::Package::WGT;
    return true;
  }

  *error = base::StringPrintf("%s", errors::kManifestUnreadable);
  return false;
}

}  // namespace

namespace xwalk {
namespace application {

static base::LazyInstance<ConfigXMLLoader> config_xml_loader =
    LAZY_INSTANCE_INITIALIZER;

scoped_refptr<ApplicationData> LoadApplication(
    const base::FilePath& application_path,
    Manifest::SourceType source_type,
    std::string* error) {
  Package::Type package_type;
  if (!GetPackageType(application_path, &package_type, error))
    return NULL;

  return LoadApplication(application_path, std::string(),
                         source_type, package_type, error);
}

scoped_refptr<ApplicationData> LoadApplication(
    const base::FilePath& application_path,
    const std::string& application_id,
    Manifest::SourceType source_type,
    Package::Type package_type,
    std::string* error) {
  scoped_ptr<base::DictionaryValue> manifest(
      LoadManifest(application_path, package_type, error));
  if (!manifest.get())
    return NULL;

  scoped_refptr<ApplicationData> application = ApplicationData::Create(
                                                             application_path,
                                                             source_type,
                                                             *manifest,
                                                             application_id,
                                                             error);
  if (!application.get())
    return NULL;

  std::vector<InstallWarning> warnings;
  ManifestHandlerRegistry* registry =
      manifest->HasKey(widget_keys::kWidgetKey)
      ? ManifestHandlerRegistry::GetInstance(Package::WGT)
      : ManifestHandlerRegistry::GetInstance(Package::XPK);

  if (!registry->ValidateAppManifest(application, error, &warnings))
    return NULL;

  if (!warnings.empty()) {
    LOG(WARNING) << "There are some warnings when validating the application "
                 << application->ID();
  }

  return application;
}

static base::DictionaryValue* LoadManifestXpk(
    const base::FilePath& manifest_path,
    std::string* error) {
  JSONFileValueSerializer serializer(manifest_path);
  scoped_ptr<base::Value> root(serializer.Deserialize(NULL, error));
  if (!root.get()) {
    if (error->empty()) {
      // If |error| is empty, than the file could not be read.
      // It would be cleaner to have the JSON reader give a specific error
      // in this case, but other code tests for a file error with
      // error->empty().  For now, be consistent.
      *error = base::StringPrintf("%s", errors::kManifestUnreadable);
    } else {
      *error = base::StringPrintf("%s  %s",
                                  errors::kManifestParseError,
                                  error->c_str());
    }
    return NULL;
  }

  if (!root->IsType(base::Value::TYPE_DICTIONARY)) {
    *error = base::StringPrintf("%s", errors::kManifestUnreadable);
    return NULL;
  }

  base::DictionaryValue* dv =
      static_cast<base::DictionaryValue*>(root.release());
#if defined(OS_TIZEN)
  // Ignore any Tizen application ID, as this is automatically generated.
  dv->Remove(keys::kTizenAppIdKey, NULL);
#endif

  return dv;
}

static base::DictionaryValue* LoadManifestWgt(
    const base::FilePath& manifest_path,
    std::string* error) {
  return config_xml_loader.Get().Load(manifest_path, error);
}

base::DictionaryValue* LoadManifest(const base::FilePath& application_path,
                                    Package::Type package_type,
                                    std::string* error) {
  base::FilePath manifest_path;

  manifest_path = application_path.Append(kManifestXpkFilename);
  if (package_type == Package::XPK)
    return LoadManifestXpk(manifest_path, error);

  manifest_path = application_path.Append(kManifestWgtFilename);
  if (package_type == Package::WGT)
    return LoadManifestWgt(manifest_path, error);

  *error = base::StringPrintf("%s", errors::kManifestUnreadable);
  return NULL;
}

base::FilePath ApplicationURLToRelativeFilePath(const GURL& url) {
  std::string url_path = url.path();
  if (url_path.empty() || url_path[0] != '/')
    return base::FilePath();

  // Drop the leading slashes and convert %-encoded UTF8 to regular UTF8.
  std::string file_path = net::UnescapeURLComponent(url_path,
      net::UnescapeRule::SPACES | net::UnescapeRule::URL_SPECIAL_CHARS);
  size_t skip = file_path.find_first_not_of("/\\");
  if (skip != file_path.npos)
    file_path = file_path.substr(skip);

  base::FilePath path =
#if defined(OS_POSIX)
    base::FilePath(file_path);
#elif defined(OS_WIN)
    base::FilePath(base::UTF8ToWide(file_path));
#else
    base::FilePath();
    NOTIMPLEMENTED();
#endif

  // It's still possible for someone to construct an annoying URL whose path
  // would still wind up not being considered relative at this point.
  // For example: app://id/c:////foo.html
  if (path.IsAbsolute())
    return base::FilePath();

  return path;
}

}  // namespace application
}  // namespace xwalk
