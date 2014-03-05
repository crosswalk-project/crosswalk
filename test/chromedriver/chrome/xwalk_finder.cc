// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/chrome/xwalk_finder.h"

#include <string>
#include <vector>

#include "base/base_paths.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "build/build_config.h"

namespace {

#if defined(OS_LINUX)
void GetApplicationDirs(std::vector<base::FilePath>* locations) {
  locations->push_back(base::FilePath("/opt/crosswalk"));
  locations->push_back(base::FilePath("/usr/local/bin"));
  locations->push_back(base::FilePath("/usr/local/sbin"));
  locations->push_back(base::FilePath("/usr/bin"));
  locations->push_back(base::FilePath("/usr/sbin"));
  locations->push_back(base::FilePath("/bin"));
  locations->push_back(base::FilePath("/sbin"));
}
#endif

}  // namespace

namespace internal {

bool FindExe(
    const base::Callback<bool(const base::FilePath&)>& exists_func,
    const std::vector<base::FilePath>& rel_paths,
    const std::vector<base::FilePath>& locations,
    base::FilePath* out_path) {
  for (size_t i = 0; i < rel_paths.size(); ++i) {
    for (size_t j = 0; j < locations.size(); ++j) {
      base::FilePath path = locations[j].Append(rel_paths[i]);
      if (exists_func.Run(path)) {
        *out_path = path;
        return true;
      }
    }
  }
  return false;
}

}  // namespace internal

bool FindXwalk(base::FilePath* browser_exe) {
#if defined(OS_LINUX)
  base::FilePath browser_exes_array[] = {
      base::FilePath("xwalk")
  };
#endif
  std::vector<base::FilePath> browser_exes(
      browser_exes_array, browser_exes_array + arraysize(browser_exes_array));
  base::FilePath module_dir;
  if (PathService::Get(base::DIR_MODULE, &module_dir)) {
    for (size_t i = 0; i < browser_exes.size(); ++i) {
      base::FilePath path = module_dir.Append(browser_exes[i]);
      if (base::PathExists(path)) {
        *browser_exe = path;
        return true;
      }
    }
  }

  std::vector<base::FilePath> locations;
  GetApplicationDirs(&locations);
  return internal::FindExe(
      base::Bind(&base::PathExists),
      browser_exes,
      locations,
      browser_exe);
}
