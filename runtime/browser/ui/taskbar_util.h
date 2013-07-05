// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_RUNTIME_BROWSER_UI_TASKBAR_UTIL_H_
#define CAMEO_RUNTIME_BROWSER_UI_TASKBAR_UTIL_H_

namespace xwalk {

// Set the ID for current process so that the icons of different apps
// on status bar will show in different group.
void SetTaskbarGroupIdForProcess();

}  // namespace xwalk

#endif  // CAMEO_RUNTIME_BROWSER_UI_TASKBAR_UTIL_H_
