// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_PEPPER_XWALK_RENDERER_PEPPER_HOST_FACTORY_H_
#define XWALK_RUNTIME_RENDERER_PEPPER_XWALK_RENDERER_PEPPER_HOST_FACTORY_H_

#include "ppapi/host/host_factory.h"

namespace content {
class RendererPpapiHost;
}

class XWalkRendererPepperHostFactory : public ppapi::host::HostFactory {
 public:
  explicit XWalkRendererPepperHostFactory(content::RendererPpapiHost* host);
  virtual ~XWalkRendererPepperHostFactory();

  // HostFactory.
  virtual scoped_ptr<ppapi::host::ResourceHost> CreateResourceHost(
      ppapi::host::PpapiHost* host,
      const ppapi::proxy::ResourceMessageCallParams& params,
      PP_Instance instance,
      const IPC::Message& message) OVERRIDE;

 private:
  // Not owned by this object.
  content::RendererPpapiHost* host_;

  DISALLOW_COPY_AND_ASSIGN(XWalkRendererPepperHostFactory);
};

#endif  // XWALK_RUNTIME_RENDERER_PEPPER_XWALK_RENDERER_PEPPER_HOST_FACTORY_H_
