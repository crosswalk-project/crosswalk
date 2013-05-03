// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_SHELL_TEST_CONFIGURATION_H_
#define CONTENT_SHELL_SHELL_TEST_CONFIGURATION_H_

#include <string>

#include "base/files/file_path.h"
#include "googleurl/src/gurl.h"

namespace content {

struct ShellTestConfiguration {
  ShellTestConfiguration();
  ~ShellTestConfiguration();

  // The current working directory.
  base::FilePath current_working_directory;

  // The temporary directory of the system.
  base::FilePath temp_path;

  // The URL of the current layout test.
  GURL test_url;

  // True if pixel tests are enabled.
  bool enable_pixel_dumping;

  // The layout test timeout in milliseconds.
  int layout_test_timeout;

  // True if tests can open external URLs
  bool allow_external_pages;

  // The expected MD5 hash of the pixel results.
  std::string expected_pixel_hash;
};

}  // namespace content

#endif  // CONTENT_SHELL_SHELL_TEST_CONFIGURATION_H_
