// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_
#define XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_

int xwalk_appcore_init(int argc, char** argv, const char* name);

int xwalk_change_cmdline(int argc, char** argv, const char* app_id);

char* xwalk_extract_app_id(const char* tizen_app_id);

#endif  // XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_
