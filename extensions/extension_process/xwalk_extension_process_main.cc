// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/extension_process/xwalk_extension_process_main.h"

#include "base/logging.h"
#include "xwalk/extensions/extension_process/xwalk_extension_process.h"

int XWalkExtensionProcessMain(const content::MainFunctionParams& parameters) {
  VLOG(0) << "Extension process running!";
  xwalk::extensions::XWalkExtensionProcess extension_process;
  extension_process.Run();
  return 0;
}
