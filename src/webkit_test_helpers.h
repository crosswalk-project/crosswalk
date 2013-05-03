// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_WEBKIT_TEST_HELPERS_H_
#define CONTENT_SHELL_WEBKIT_TEST_HELPERS_H_

namespace WebTestRunner {
struct WebPreferences;
}

namespace webkit_glue {
struct WebPreferences;
}

namespace content {

// The TestRunner library keeps its settings in a WebTestRunner::WebPreferenes
// object. The content_shell, however, uses webkit_glue::WebPreferences. This
// method exports the settings from the WebTestRunner library which are relevant
// for layout tests.
void ExportLayoutTestSpecificPreferences(
    const WebTestRunner::WebPreferences& from, webkit_glue::WebPreferences* to);

// Applies settings that differ between layout tests and regular mode.
void ApplyLayoutTestDefaultPreferences(webkit_glue::WebPreferences* prefs);

}  // namespace content

#endif  // CONTENT_SHELL_WEBKIT_TEST_HELPERS_H_
