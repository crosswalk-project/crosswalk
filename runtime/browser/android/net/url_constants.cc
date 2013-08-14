// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/net/url_constants.h"

namespace xwalk {

// The content: scheme is used in Android for interacting with content
// provides.
// See http://developer.android.com/reference/android/content/ContentUris.html
const char kContentScheme[] = "content";

// These are special paths used with the file: scheme to access application
// assets and resources.
// See http://developer.android.com/reference/android/webkit/WebSettings.html
const char kAndroidAssetPath[] = "/android_asset/";
const char kAndroidResourcePath[] = "/android_res/";

}  // namespace xwalk
