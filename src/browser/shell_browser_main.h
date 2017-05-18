// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_SHELL_BROWSER_MAIN_H_
#define CAMEO_SRC_BROWSER_SHELL_BROWSER_MAIN_H_

namespace content {
struct MainFunctionParams;
}

int ShellBrowserMain(const content::MainFunctionParams& parameters);

#endif  // CAMEO_SRC_BROWSER_SHELL_BROWSER_MAIN_H_
