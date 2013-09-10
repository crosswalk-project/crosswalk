// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_

#include "net/url_request/url_request_job_factory.h"

class ExtensionInfoMap;

// Creates the handlers for the chrome-extension:// scheme.
net::URLRequestJobFactory::ProtocolHandler* CreateExtensionProtocolHandler(
    bool is_incognito,
    ExtensionInfoMap* extension_info_map);

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_
