// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_COMMON_CAMEO_PATHS_H_
#define CAMEO_SRC_RUNTIME_COMMON_CAMEO_PATHS_H_

#include "build/build_config.h"
#include "content/public/browser/notification_types.h"

namespace cameo {

enum {
  PATH_START = 1000,
  DIR_DATA_PATH = PATH_START,  // Directory where the cache and local storage
                               // data resides.
  DIR_TEST_DATA,               // Directory where unit test data resides.
  PATH_END
};

void RegisterPathProvider();

}  // namespace cameo

#endif  // CAMEO_SRC_RUNTIME_COMMON_CAMEO_PATHS_H_
