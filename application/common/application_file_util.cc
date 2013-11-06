// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_file_util.h"

#include <map>
#include <vector>

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/json/json_file_value_serializer.h"
#include "base/logging.h"
#include "base/metrics/histogram.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/install_warning.h"
#include "xwalk/application/common/manifest_handler.h"
#include "net/base/escape.h"
#include "net/base/file_stream.h"
#include "ui/base/l10n/l10n_util.h"

namespace errors = xwalk::application_manifest_errors;

namespace xwalk {
namespace application {

scoped_refptr<Application> LoadApplication(
    const base::FilePath& application_path,
    Manifest::SourceType source_type,
    std::string* error) {
  return LoadApplication(application_path, std::string(),
                         source_type, error);
}

scoped_refptr<Application> LoadApplication(
    const base::FilePath& application_path,
    const std::string& application_id,
    Manifest::SourceType source_type,
    std::string* error) {
  scoped_ptr<DictionaryValue> manifest(LoadManifest(application_path, error));
  if (!manifest.get())
    return NULL;

  scoped_refptr<Application> application = Application::Create(application_path,
                                                             source_type,
                                                             *manifest,
                                                             application_id,
                                                             error);
  if (!application.get())
    return NULL;

  std::vector<InstallWarning> warnings;
  if (!ManifestHandlerRegistry::GetInstance()->ValidateAppManifest(
          application, error, &warnings))
    return NULL;
  if (!warnings.empty()) {
    LOG(WARNING) << "There are some warnings when validating the application "
                 << application->ID();
  }

  return application;
}

DictionaryValue* LoadManifest(const base::FilePath& application_path,
                              std::string* error) {
  base::FilePath manifest_path =
      application_path.Append(kManifestFilename);
  if (!base::PathExists(manifest_path)) {
    *error = base::StringPrintf("%s",
                                errors::kManifestUnreadable);
    return NULL;
  }

  JSONFileValueSerializer serializer(manifest_path);
  scoped_ptr<Value> root(serializer.Deserialize(NULL, error));
  if (!root.get()) {
    if (error->empty()) {
      // If |error| is empty, than the file could not be read.
      // It would be cleaner to have the JSON reader give a specific error
      // in this case, but other code tests for a file error with
      // error->empty().  For now, be consistent.
      *error = base::StringPrintf("%s",
                                  errors::kManifestUnreadable);
    } else {
      *error = base::StringPrintf("%s  %s",
                                  errors::kManifestParseError,
                                  error->c_str());
    }
    return NULL;
  }

  if (!root->IsType(Value::TYPE_DICTIONARY)) {
    *error = base::StringPrintf("%s",
                                errors::kManifestUnreadable);
    return NULL;
  }

  return static_cast<DictionaryValue*>(root.release());
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
    base::FilePath(UTF8ToWide(file_path));
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
