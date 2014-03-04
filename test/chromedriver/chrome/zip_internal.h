// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_ZIP_INTERNAL_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_ZIP_INTERNAL_H_

#if defined(OS_WIN)
#include <windows.h>
#endif

#include <string>

#if defined(USE_SYSTEM_MINIZIP)
#include <minizip/unzip.h>
#include <minizip/zip.h>
#else
#include "third_party/zlib/contrib/minizip/unzip.h"
#include "third_party/zlib/contrib/minizip/zip.h"
#endif

// Utility functions and constants used internally for the zip file
// library in the directory. Don't use them outside of the library.
namespace zip {
namespace internal {

// Opens the given file name in UTF-8 for unzipping, with some setup for
// Windows.
unzFile OpenForUnzipping(const std::string& file_name_utf8);

#if defined(OS_POSIX)
// Opens the file referred to by |zip_fd| for unzipping.
unzFile OpenFdForUnzipping(int zip_fd);
#endif

#if defined(OS_WIN)
// Opens the file referred to by |zip_handle| for unzipping.
unzFile OpenHandleForUnzipping(HANDLE zip_handle);
#endif

// Creates a custom unzFile object which reads data from the specified string.
// This custom unzFile object overrides the I/O API functions of zlib so it can
// read data from the specified string.
unzFile PreprareMemoryForUnzipping(const std::string& data);

// Opens the given file name in UTF-8 for zipping, with some setup for
// Windows. |append_flag| will be passed to zipOpen2().
zipFile OpenForZipping(const std::string& file_name_utf8, int append_flag);

#if defined(OS_POSIX)
// Opens the file referred to by |zip_fd| for zipping. |append_flag| will be
// passed to zipOpen2().
zipFile OpenFdForZipping(int zip_fd, int append_flag);
#endif

const int kZipMaxPath = 256;
const int kZipBufSize = 8192;

}  // namespace internal
}  // namespace zip

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_ZIP_INTERNAL_H_
