// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_BASE_XWALK_TEST_UTILS_H_
#define XWALK_TEST_BASE_XWALK_TEST_UTILS_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "url/gurl.h"

namespace xwalk {
class Runtime;
}

class CommandLine;

// A set of utilities for test code that launches separate processes.
namespace xwalk_test_utils {

// Appends browser switches to provided |command_line| to be used
// when running under tests.
void PrepareBrowserCommandLineForTests(CommandLine* command_line);

// Override the data path for testing.
bool OverrideDataPathDir(const base::FilePath& data_path_dir);

// Generate the URL for testing a particular test.
// HTML for the tests is all located in test_data_directory/<dir>/<file>
// The returned path is GURL format.
GURL GetTestURL(const base::FilePath& dir, const base::FilePath& file);

// Get the file path for test file under test_data_directory.
base::FilePath GetTestFilePath(const base::FilePath& dir,
                               const base::FilePath& file);

// Navigate a specified URL in the given Runtime. It will block until the
// navigation completes.
void NavigateToURL(xwalk::Runtime* runtime, const GURL& url);

}  // namespace xwalk_test_utils

#endif  // XWALK_TEST_BASE_XWALK_TEST_UTILS_H_
