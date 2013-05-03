// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/shell_application_mac.h"

#include "base/auto_reset.h"
#include "cameo/src/browser/shell.h"
#include "cameo/src/browser/shell_browser_context.h"
#include "cameo/src/browser/shell_content_browser_client.h"
#include "googleurl/src/gurl.h"

@implementation ShellCrApplication

- (BOOL)isHandlingSendEvent {
  return handlingSendEvent_;
}

- (void)sendEvent:(NSEvent*)event {
  base::AutoReset<BOOL> scoper(&handlingSendEvent_, YES);
  [super sendEvent:event];
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  handlingSendEvent_ = handlingSendEvent;
}

- (IBAction)newDocument:(id)sender {
  cameo::ShellBrowserContext* browserContext =
      cameo::ShellContentBrowserClient::Get()->browser_context();
  cameo::Shell::CreateNewWindow(browserContext,
                                GURL("about:blank"),
                                NULL,
                                MSG_ROUTING_NONE,
                                gfx::Size());
}

@end
