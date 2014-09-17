// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_FILE_UTIL_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_FILE_UTIL_H_

#include <string>
#include <map>

#include "base/memory/ref_counted.h"
#include "xwalk/application/common/application_data.h"

class GURL;

namespace base {
class DictionaryValue;
class FilePath;
}

// Utilities for manipulating the on-disk storage of applications.
namespace xwalk {
namespace application {

class FileDeleter {
 public:
  FileDeleter(const base::FilePath& path, bool recursive);
  ~FileDeleter();

  void Dismiss() { path_.clear(); }
  const base::FilePath& path() const { return path_; }

 private:
  base::FilePath path_;
  bool recursive_;
};

// Loads an application manifest from the specified directory. Returns NULL
// on failure, with a description of the error in |error|.
scoped_ptr<Manifest> LoadManifest(
    const base::FilePath& file_path, Manifest::Type type, std::string* error);

base::FilePath GetManifestPath(
    const base::FilePath& app_directory, Manifest::Type type);

// Loads and validates an application from the specified directory. Returns NULL
// on failure, with a description of the error in |error|.
scoped_refptr<ApplicationData> LoadApplication(
    const base::FilePath& app_root, const std::string& app_id,
    ApplicationData::SourceType source_type, Manifest::Type manifest_type,
    std::string* error);

// Get a relative file path from an app:// URL.
base::FilePath ApplicationURLToRelativeFilePath(const GURL& url);

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_FILE_UTIL_H_
