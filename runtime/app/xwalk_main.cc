// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/xwalk_main_delegate.h"
#include "content/public/app/content_main.h"

#if defined(OS_MACOSX)
#include "xwalk/runtime/app/xwalk_content_main.h"
#endif

#if defined(OS_WIN)
#include "content/public/app/startup_helper_win.h"
#include "sandbox/win/src/sandbox_types.h"
#endif

#if defined(OS_WIN)
int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
#else
int main(int argc, const char** argv) {
#endif
#if defined(OS_MACOSX)
    // Do the delegate work in xwalk_content_main to avoid having to export the
  // delegate types.
  return ::ContentMain(argc, argv);
#else
  xwalk::XWalkMainDelegate delegate;
  content::ContentMainParams params(&delegate);

#if defined(OS_WIN)
  sandbox::SandboxInterfaceInfo sandbox_info = {0};
  content::InitializeSandboxInfo(&sandbox_info);
  params.instance = instance;
  params.sandbox_info = &sandbox_info;
#else
  params.argc = argc;
  params.argv = argv;
#endif
  return content::ContentMain(params);
#endif
}
