// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/native_file_system/virtual_root_provider.h"

#include <tzplatform_config.h>

#include <map>
#include <string>

VirtualRootProvider::VirtualRootProvider() {
  const char* names[] = {
      "CAMERA",
      "DOCUMENTS",
      "IMAGES",
      "SOUNDS",
      "VIDEOS",
  };

  tzplatform_variable dirs[] = {
      TZ_USER_CAMERA,
      TZ_USER_DOCUMENTS,
      TZ_USER_IMAGES,
      TZ_USER_SOUNDS,
      TZ_USER_VIDEOS
  };

  for (unsigned int i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
    virtual_root_map_[names[i]] =
        base::FilePath::FromUTF8Unsafe(
            std::string(tzplatform_getenv(dirs[i])));
  }

  virtual_root_map_["RINGTONES"] =
      base::FilePath::FromUTF8Unsafe(
          std::string(tzplatform_mkpath(TZ_USER_SHARE, "settings/Ringtones")));
}
