// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/xwalk_views_delegate.h"

#if !defined(OS_WIN) && defined(USE_AURA)

namespace xwalk {

XWalkViewsDelegate::XWalkViewsDelegate() {}

XWalkViewsDelegate::~XWalkViewsDelegate() {}

// FIXME(cmarcelo): Needed?
bool XWalkViewsDelegate::UseTransparentWindows() const {
  return false;
}

}  // namespace xwalk

#endif
