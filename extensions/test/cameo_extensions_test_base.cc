// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/extensions/test/cameo_extensions_test_base.h"

#include "base/path_service.h"
#include "base/utf_string_conversions.h"
#include "cameo/extensions/browser/cameo_extension_service.h"
#include "cameo/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"

using cameo::extensions::CameoExtensionService;

void CameoExtensionsTestBase::SetUp() {
  CameoExtensionService::SetRegisterExtensionsCallbackForTesting(
      base::Bind(&CameoExtensionsTestBase::RegisterExtensions,
                 base::Unretained(this)));
  InProcessBrowserTest::SetUp();
}

GURL GetExtensionsTestURL(const base::FilePath& dir,
                          const base::FilePath& file) {
  base::FilePath test_file;
  PathService::Get(base::DIR_SOURCE_ROOT, &test_file);
  test_file = test_file
              .Append(FILE_PATH_LITERAL("cameo"))
              .Append(FILE_PATH_LITERAL("extensions"))
              .Append(FILE_PATH_LITERAL("test"))
              .Append(FILE_PATH_LITERAL("data"))
              .Append(dir).Append(file);
  return net::FilePathToFileURL(test_file);
}
