// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_TEST_XWALK_EXTENSIONS_TEST_BASE_H_
#define XWALK_EXTENSIONS_TEST_XWALK_EXTENSIONS_TEST_BASE_H_

#include "base/strings/utf_string_conversions.h"
#include "xwalk/test/base/in_process_browser_test.h"

namespace xwalk {
namespace extensions {
class XWalkExtensionServer;
class XWalkExtensionService;
}
}

class XWalkExtensionsTestBase : public InProcessBrowserTest {
 public:
  virtual void SetUp() OVERRIDE;
  virtual void RegisterExtensions(
      xwalk::extensions::XWalkExtensionService* extension_service,
      xwalk::extensions::XWalkExtensionServer* server) {}
};

GURL GetExtensionsTestURL(const base::FilePath& dir,
                          const base::FilePath& file);

const string16 kPassString = ASCIIToUTF16("Pass");
const string16 kFailString = ASCIIToUTF16("Fail");

#endif  // XWALK_EXTENSIONS_TEST_XWALK_EXTENSIONS_TEST_BASE_H_
