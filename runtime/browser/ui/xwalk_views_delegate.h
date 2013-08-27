// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_XWALK_VIEWS_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_UI_XWALK_VIEWS_DELEGATE_H_

#if !defined(OS_WIN) && defined(USE_AURA)

#include "ui/views/test/desktop_test_views_delegate.h"

namespace xwalk {

// TODO(cmarcelo): Remove usage of the test views delegate.

// Views delegate implementation for Crosswalk. Controls application-wide
// aspects of Views toolkit system.
class XWalkViewsDelegate : public views::DesktopTestViewsDelegate {
 public:
  XWalkViewsDelegate();
  virtual ~XWalkViewsDelegate();

  // views::TestViewsDelegate implementation.
  virtual bool UseTransparentWindows() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkViewsDelegate);
};

}  // namespace xwalk

#endif  // !defined(OS_WIN) && defined(USE_AURA)

#endif  // XWALK_RUNTIME_BROWSER_UI_XWALK_VIEWS_DELEGATE_H_
