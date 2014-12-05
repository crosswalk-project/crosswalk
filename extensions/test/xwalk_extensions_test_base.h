// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_TEST_XWALK_EXTENSIONS_TEST_BASE_H_
#define XWALK_EXTENSIONS_TEST_XWALK_EXTENSIONS_TEST_BASE_H_

#include <vector>
#include "base/strings/utf_string_conversions.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"

class XWalkExtensionsTestBase : public InProcessBrowserTest {
 public:
  void SetUp() override;

  virtual void CreateExtensionsForUIThread(
      xwalk::extensions::XWalkExtensionVector* extensions);
  virtual void CreateExtensionsForExtensionThread(
      xwalk::extensions::XWalkExtensionVector* extensions);
};

GURL GetExtensionsTestURL(const base::FilePath& dir,
                          const base::FilePath& file);

base::FilePath GetExternalExtensionTestPath(
    const base::FilePath::CharType test[]);

const base::string16 kPassString = base::ASCIIToUTF16("Pass");
const base::string16 kFailString = base::ASCIIToUTF16("Fail");

#endif  // XWALK_EXTENSIONS_TEST_XWALK_EXTENSIONS_TEST_BASE_H_
