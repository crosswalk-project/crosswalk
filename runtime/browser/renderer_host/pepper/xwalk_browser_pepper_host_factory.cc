// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/renderer_host/pepper/xwalk_browser_pepper_host_factory.h"

#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"

using ppapi::host::ResourceHost;

namespace xwalk {

XWalkBrowserPepperHostFactory::XWalkBrowserPepperHostFactory(
    content::BrowserPpapiHost* host)
    : host_(host) {
}

XWalkBrowserPepperHostFactory::~XWalkBrowserPepperHostFactory() {
}

scoped_ptr<ResourceHost> XWalkBrowserPepperHostFactory::CreateResourceHost(
    ppapi::host::PpapiHost* host,
    const ppapi::proxy::ResourceMessageCallParams& params,
    PP_Instance instance,
    const IPC::Message& message) {
  DCHECK(host == host_->GetPpapiHost());

  return scoped_ptr<ResourceHost>();
}

}  // namespace xwalk
