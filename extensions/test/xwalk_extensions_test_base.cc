// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include <vector>
#include "base/path_service.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"

using xwalk::extensions::XWalkExtensionVector;
using xwalk::extensions::XWalkExtensionService;

void XWalkExtensionsTestBase::SetUp() {
  XWalkExtensionService::SetCreateUIThreadExtensionsCallbackForTesting(
      base::Bind(&XWalkExtensionsTestBase::CreateExtensionsForUIThread,
                 base::Unretained(this)));
  XWalkExtensionService::SetCreateExtensionThreadExtensionsCallbackForTesting(
      base::Bind(&XWalkExtensionsTestBase::CreateExtensionsForExtensionThread,
                 base::Unretained(this)));
  InProcessBrowserTest::SetUp();
}

void XWalkExtensionsTestBase::CreateExtensionsForUIThread(
    XWalkExtensionVector* extensions) {}

void XWalkExtensionsTestBase::CreateExtensionsForExtensionThread(
    XWalkExtensionVector* extensions) {}

GURL GetExtensionsTestURL(const base::FilePath& dir,
                          const base::FilePath& file) {
  base::FilePath test_file;
  PathService::Get(base::DIR_SOURCE_ROOT, &test_file);
  test_file = test_file
              .Append(FILE_PATH_LITERAL("xwalk"))
              .Append(FILE_PATH_LITERAL("extensions"))
              .Append(FILE_PATH_LITERAL("test"))
              .Append(FILE_PATH_LITERAL("data"))
              .Append(dir).Append(file);
  return net::FilePathToFileURL(test_file);
}

base::FilePath GetExternalExtensionTestPath(
    const base::FilePath::CharType test[]) {
  base::FilePath extension_dir;
  PathService::Get(base::DIR_EXE, &extension_dir);
  extension_dir = extension_dir
                  .Append(FILE_PATH_LITERAL("tests"))
                  .Append(FILE_PATH_LITERAL("extension"))
                  .Append(test);
  return extension_dir;
}
