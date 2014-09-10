// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_PATHS_H_
#define XWALK_RUNTIME_COMMON_XWALK_PATHS_H_

#include "build/build_config.h"

namespace xwalk {

enum {
  PATH_START = 1000,
  DIR_DATA_PATH = PATH_START,  // Directory where the cache and local storage
                               // data resides.
  DIR_INTERNAL_PLUGINS,        // Directory where internal plugins reside.

  FILE_NACL_PLUGIN,            // Full path to the internal NaCl plugin file.
  DIR_PNACL_COMPONENT,         // Full path to the latest PNaCl version
                               // (subdir of DIR_PNACL_BASE).
  DIR_TEST_DATA,               // Directory where unit test data resides.
  DIR_WGT_STORAGE_PATH,        // Directory where widget storage data resides.
  DIR_APPLICATION_PATH,        // Directory where applications data is stored.
  PATH_END
};

void RegisterPathProvider();

}  // namespace xwalk

#endif  // XWALK_RUNTIME_COMMON_XWALK_PATHS_H_
