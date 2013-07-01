// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/app/cameo_main_delegate.h"
#include "content/public/app/content_main.h"

#if defined(OS_WIN)
#include "content/public/app/startup_helper_win.h"
#include "sandbox/win/src/sandbox_types.h"
#endif

#if defined(OS_WIN)
int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
  sandbox::SandboxInterfaceInfo sandbox_info = {0};
  content::InitializeSandboxInfo(&sandbox_info);
  cameo::CameoMainDelegate delegate;
  return content::ContentMain(instance, &sandbox_info, &delegate);
}
#elif defined(OS_LINUX)
int main(int argc, const char** argv) {
  cameo::CameoMainDelegate delegate;
  return content::ContentMain(argc, argv, &delegate);
}
#else
#error "Unsupport platform."
#endif
