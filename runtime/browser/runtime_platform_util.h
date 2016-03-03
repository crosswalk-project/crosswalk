// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_PLATFORM_UTIL_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_PLATFORM_UTIL_H_

#include <string>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "ui/gfx/native_widget_types.h"

class GURL;

namespace base {
class FilePath;
}

namespace platform_util {
// Type of item that is the target of the OpenItem() call.
enum OpenItemType { OPEN_FILE, OPEN_FOLDER };

// Opens the item specified by |full_path|, which is expected to be the type
// indicated by |item_type| in the desktop's default manner.
// Must be called on the UI thread.
void OpenItem(const base::FilePath& full_path, OpenItemType item_type);

// Opens the folder containing the item specified by |full_path| in the
// desktop's default manner. If possible, the item will be selected. Must
// be called on the UI thread.
void ShowItemInFolder(const base::FilePath& full_path);

// Open the given external protocol URL in the desktop's default manner.
// (For example, mailto: URLs in the default mail user agent.)
void OpenExternal(const GURL& url);

// Get the top level window for the native view. This can return NULL.
gfx::NativeWindow GetTopLevel(gfx::NativeView view);

// Get the direct parent of |view|, may return NULL.
gfx::NativeView GetParent(gfx::NativeView view);

// Returns true if |window| is the foreground top level window.
bool IsWindowActive(gfx::NativeWindow window);

// Activate the window, bringing it to the foreground top level.
void ActivateWindow(gfx::NativeWindow window);

// Returns true if the view is visible. The exact definition of this is
// platform-specific, but it is generally not "visible to the user", rather
// whether the view has the visible attribute set.
bool IsVisible(gfx::NativeView view);

// Set preferred screen orientation.
// Preferred means that if desired orientation is not available e.g. in laptop
// mode, it is still set to be activated once the machine transforms to tablet
// mode.
// orientation is defined in manifest - could take any of the values from here
// https://crosswalk-project.org/documentation/manifest/orientation.html
void SetPreferredScreenOrientation(const std::string& orientation);

#if defined(OS_MACOSX)
// On 10.7+, back and forward swipe gestures can be triggered using a scroll
// gesture, if enabled in System Preferences. This function returns true if
// the feature is supported and enabled, and false otherwise.
bool IsSwipeTrackingFromScrollEventsEnabled();
#endif
}  // namespace platform_util

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_PLATFORM_UTIL_H_
