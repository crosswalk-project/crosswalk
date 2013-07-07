// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_IMAGE_UTIL_H_
#define XWALK_RUNTIME_BROWSER_IMAGE_UTIL_H_

#include "base/files/file_path.h"
#include "ui/gfx/image/image.h"

namespace xwalk_utils {

// Load a gfx::Image from a PNG file or ICO file.
gfx::Image LoadImageFromFilePath(const base::FilePath& filename);

}  // namespace xwalk_utils

#endif  // XWALK_RUNTIME_BROWSER_IMAGE_UTIL_H_
