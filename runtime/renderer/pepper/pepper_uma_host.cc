// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/pepper/pepper_uma_host.h"

#include "base/metrics/histogram.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/proxy/ppapi_messages.h"

PepperUMAHost::PepperUMAHost(
    content::RendererPpapiHost* host,
    PP_Instance instance,
    PP_Resource resource)
    : ResourceHost(host->GetPpapiHost(), instance, resource),
      document_url_(host->GetDocumentURL(instance)),
      is_plugin_in_process_(host->IsRunningInProcess()) {
}

PepperUMAHost::~PepperUMAHost() {
}

int32_t PepperUMAHost::OnResourceMessageReceived(
    const IPC::Message& msg,
    ppapi::host::HostMessageContext* context) {
  IPC_BEGIN_MESSAGE_MAP(PepperUMAHost, msg)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_UMA_HistogramCustomTimes,
        OnHistogramCustomTimes);
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_UMA_HistogramCustomCounts,
        OnHistogramCustomCounts);
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_UMA_HistogramEnumeration,
        OnHistogramEnumeration);
  IPC_END_MESSAGE_MAP()
  return PP_ERROR_FAILED;
}

bool PepperUMAHost::IsHistogramAllowed(const std::string& histogram) {
  return (is_plugin_in_process_ && histogram.find("NaCl.") == 0);
}

#define RETURN_IF_BAD_ARGS(_min, _max, _buckets) \
  do { \
    if (_min >= _max || _buckets <= 1) \
      return PP_ERROR_BADARGUMENT; \
  } while (0)

int32_t PepperUMAHost::OnHistogramCustomTimes(
    ppapi::host::HostMessageContext* context,
    const std::string& name,
    int64_t sample,
    int64_t min,
    int64_t max,
    uint32_t bucket_count) {
  if (!IsHistogramAllowed(name)) {
    return PP_ERROR_NOACCESS;
  }
  RETURN_IF_BAD_ARGS(min, max, bucket_count);

  base::HistogramBase* counter =
      base::Histogram::FactoryTimeGet(
          name,
          base::TimeDelta::FromMilliseconds(min),
          base::TimeDelta::FromMilliseconds(max),
          bucket_count,
          base::HistogramBase::kUmaTargetedHistogramFlag);
  counter->AddTime(base::TimeDelta::FromMilliseconds(sample));
  return PP_OK;
}

int32_t PepperUMAHost::OnHistogramCustomCounts(
    ppapi::host::HostMessageContext* context,
    const std::string& name,
    int32_t sample,
    int32_t min,
    int32_t max,
    uint32_t bucket_count) {
  if (!IsHistogramAllowed(name)) {
    return PP_ERROR_NOACCESS;
  }
  RETURN_IF_BAD_ARGS(min, max, bucket_count);

  base::HistogramBase* counter =
      base::Histogram::FactoryGet(
          name,
          min,
          max,
          bucket_count,
          base::HistogramBase::kUmaTargetedHistogramFlag);
  counter->Add(sample);
  return PP_OK;
}

int32_t PepperUMAHost::OnHistogramEnumeration(
    ppapi::host::HostMessageContext* context,
    const std::string& name,
    int32_t sample,
    int32_t boundary_value) {
  if (!IsHistogramAllowed(name)) {
    return PP_ERROR_NOACCESS;
  }
  RETURN_IF_BAD_ARGS(0, boundary_value, boundary_value + 1);

  base::HistogramBase* counter =
      base::LinearHistogram::FactoryGet(
          name,
          1,
          boundary_value,
          boundary_value + 1,
          base::HistogramBase::kUmaTargetedHistogramFlag);
  counter->Add(sample);
  return PP_OK;
}
