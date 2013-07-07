// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_COLOR_CHOOSER_H_
#define XWALK_RUNTIME_BROWSER_UI_COLOR_CHOOSER_H_

#include "content/public/browser/color_chooser.h"
#include "third_party/skia/include/core/SkColor.h"

class XWalkFormInputTest;

namespace xwalk {

class ColorChooser : public content::ColorChooser {
 public:
  explicit ColorChooser(int identifier) : content::ColorChooser(identifier) {}

  static bool IsTesting();
  static SkColor GetColorForBrowserTest();

 private:
  friend class ::XWalkFormInputTest;
  // Set the color will be chosen for test purpose
  static void SetColorForBrowserTest(SkColor color);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_COLOR_CHOOSER_H_

