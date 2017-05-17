// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_XWALK_FINDER_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_XWALK_FINDER_H_

#include <vector>

#include "base/callback_forward.h"

namespace base {
class FilePath;
}

// Gets the path to the default xwalk executable. Returns true on success.
bool FindXwalk(base::FilePath* browser_exe);

namespace internal {

bool FindExe(
    const base::Callback<bool(const base::FilePath&)>& exists_func,  // NOLINT
    const std::vector<base::FilePath>& rel_paths,
    const std::vector<base::FilePath>& locations,
    base::FilePath* out_path);

}  // namespace internal

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_XWALK_FINDER_H_
