// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_TEST_BASE_IMAGE_UTIL_H_
#define CAMEO_SRC_TEST_BASE_IMAGE_UTIL_H_

#include "base/files/file_path.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/size.h"

namespace cameo_test_utils {

// Load a gfx::Image from a PNG file or ICO file.
gfx::Image LoadImageFromFilePath(const base::FilePath& filename);

}  // namespace cameo_test_utils

#endif  // CAMEO_SRC_TEST_BASE_IMAGE_UTIL_H_
