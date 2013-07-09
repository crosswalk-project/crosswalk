// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_APPLICATION_MAC_H_
#define XWALK_RUNTIME_BROWSER_XWALK_APPLICATION_MAC_H_

#include "base/mac/scoped_sending_event.h"

@interface XWalkCrApplication : NSApplication<CrAppProtocol,
                                              CrAppControlProtocol> {
 @private
  BOOL handlingSendEvent_;
}

// CrAppProtocol:
- (BOOL)isHandlingSendEvent;

// CrAppControlProtocol:
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent;

@end

#endif  // XWALK_RUNTIME_BROWSER_XWALK_APPLICATION_MAC_H_
