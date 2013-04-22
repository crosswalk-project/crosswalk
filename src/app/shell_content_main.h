// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_APP_SHELL_CONTENT_MAIN_H_
#define CAMEO_SRC_APP_SHELL_CONTENT_MAIN_H_

#include "base/basictypes.h"

#if defined(OS_MACOSX)
extern "C" {
__attribute__((visibility("default")))
int ContentMain(int argc,
                const char** argv);
}  // extern "C"
#endif  // OS_MACOSX

#endif  // CAMEO_SRC_APP_SHELL_CONTENT_MAIN_H_
