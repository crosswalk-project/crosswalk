// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/tizen/runtime_main.h"

#include "content/public/app/content_main.h"
#include "xwalk/runtime/app/xwalk_main_delegate.h"

namespace xwalk {

int RuntimeMain(int argc, const char** argv) {
  xwalk::XWalkMainDelegate delegate;
  return content::ContentMain(argc, argv, &delegate);
}

}  // namespace xwalk
