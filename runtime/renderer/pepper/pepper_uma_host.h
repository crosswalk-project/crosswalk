// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_PEPPER_PEPPER_UMA_HOST_H_
#define XWALK_RUNTIME_RENDERER_PEPPER_PEPPER_UMA_HOST_H_

#include <set>
#include <string>

#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/host/resource_host.h"
#include "url/gurl.h"

namespace content {
class RendererPpapiHost;
}

namespace ppapi {
namespace host {
struct HostMessageContext;
}  // namespace host
}  // namespace ppapi

class PepperUMAHost : public ppapi::host::ResourceHost {
 public:
  PepperUMAHost(content::RendererPpapiHost* host,
                PP_Instance instance,
                PP_Resource resource);

  virtual ~PepperUMAHost();

  // ppapi::host::ResourceMessageHandler implementation.
  virtual int32_t OnResourceMessageReceived(
      const IPC::Message& msg,
      ppapi::host::HostMessageContext* context) OVERRIDE;

 private:
  bool IsHistogramAllowed(const std::string& histogram);

  int32_t OnHistogramCustomTimes(
      ppapi::host::HostMessageContext* context,
      const std::string& name,
      int64_t sample,
      int64_t min,
      int64_t max,
      uint32_t bucket_count);

  int32_t OnHistogramCustomCounts(
      ppapi::host::HostMessageContext* context,
      const std::string& name,
      int32_t sample,
      int32_t min,
      int32_t max,
      uint32_t bucket_count);

  int32_t OnHistogramEnumeration(
      ppapi::host::HostMessageContext* context,
      const std::string& name,
      int32_t sample,
      int32_t boundary_value);

  const GURL document_url_;
  bool is_plugin_in_process_;

  // Set of origins that can use UMA private APIs from NaCl.
  std::set<std::string> allowed_origins_;
  // Set of histograms that can be used from this interface.
  std::set<std::string> allowed_histograms_;

  DISALLOW_COPY_AND_ASSIGN(PepperUMAHost);
};

#endif  // XWALK_RUNTIME_RENDERER_PEPPER_PEPPER_UMA_HOST_H_
