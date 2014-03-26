// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RENDERER_HOST_PEPPER_XWALK_BROWSER_PEPPER_HOST_FACTORY_H_
#define XWALK_RUNTIME_BROWSER_RENDERER_HOST_PEPPER_XWALK_BROWSER_PEPPER_HOST_FACTORY_H_

#include "ppapi/host/host_factory.h"

namespace content {
class BrowserPpapiHost;
}  // namespace content

namespace xwalk {

class XWalkBrowserPepperHostFactory : public ppapi::host::HostFactory {
 public:
  // Non-owning pointer to the filter must outlive this class.
  explicit XWalkBrowserPepperHostFactory(content::BrowserPpapiHost* host);
  virtual ~XWalkBrowserPepperHostFactory();

  virtual scoped_ptr<ppapi::host::ResourceHost> CreateResourceHost(
      ppapi::host::PpapiHost* host,
      const ppapi::proxy::ResourceMessageCallParams& params,
      PP_Instance instance,
      const IPC::Message& message) OVERRIDE;

 private:
  // Non-owning pointer.
  content::BrowserPpapiHost* host_;

  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserPepperHostFactory);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RENDERER_HOST_PEPPER_XWALK_BROWSER_PEPPER_HOST_FACTORY_H_
