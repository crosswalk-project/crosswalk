// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_

#include <string>

#include "base/files/file_path.h"
#include "base/memory/linked_ptr.h"
#include "net/url_request/url_request_job_factory.h"
#include "xwalk/application/browser/application_system.h"

namespace net {
class HttpResponseHeaders;
}

namespace xwalk {
namespace application {

class ApplicationService;
net::HttpResponseHeaders* BuildHttpHeaders(
    const std::string& content_security_policy,
    const std::string& mime_type, const std::string& method,
    const base::FilePath& file_path, const base::FilePath& relative_path,
    bool is_authority_match);

// Creates the handlers for the app:// scheme.
linked_ptr<net::URLRequestJobFactory::ProtocolHandler>
CreateApplicationProtocolHandler(ApplicationService* service);

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_
