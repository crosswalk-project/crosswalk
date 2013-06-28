// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/browser/ui/color_chooser.h"

namespace {

SkColor g_browser_test_color;
bool g_testing = false;

}  // namespace

namespace cameo {

// static
bool ColorChooser::IsTesting() {
  return g_testing;
}

// static
SkColor ColorChooser::GetColorForBrowserTest() {
  return g_browser_test_color;
}

// static
void ColorChooser::SetColorForBrowserTest(SkColor color) {
  g_testing = true;
  g_browser_test_color = color;
}

}  // namespace cameo

