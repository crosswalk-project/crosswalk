// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_PATHS_H_
#define XWALK_RUNTIME_COMMON_XWALK_PATHS_H_

#include "build/build_config.h"
#include "content/public/browser/notification_types.h"

namespace xwalk {

enum {
  PATH_START = 1000,
  DIR_DATA_PATH = PATH_START,  // Directory where the cache and local storage
                               // data resides.
  DIR_TEST_DATA,               // Directory where unit test data resides.
  PATH_END
};

void RegisterPathProvider();

}  // namespace xwalk

#endif  // XWALK_RUNTIME_COMMON_XWALK_PATHS_H_
