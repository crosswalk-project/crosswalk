#include "cameo/cameo_browser_context.h"

#include "cameo/cameo_resource_context.h"
#include "cameo/cameo_url_request_context_getter.h"
#include "content/public/browser/storage_partition.h"
#include "net/url_request/url_request.h"

CameoBrowserContext::CameoBrowserContext()
    : resource_context_(new CameoResourceContext) {
}

CameoBrowserContext::~CameoBrowserContext() {
}

net::URLRequestContextGetter* CameoBrowserContext::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers) {
  DCHECK(!url_request_getter_);
  url_request_getter_ = new CameoURLRequestContextGetter(GetPath(),
                                                         protocol_handlers);
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

base::FilePath CameoBrowserContext::GetPath() {
  // FIXME: Find a proper path using Chromium classes.
  return base::FilePath("/tmp/cameo-rt");
}

net::URLRequestContextGetter* CameoBrowserContext::GetRequestContext() {
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
CameoBrowserContext::GetRequestContextForRenderProcess(int renderer_child_id) {
  return GetRequestContext();
}

net::URLRequestContextGetter* CameoBrowserContext::GetMediaRequestContext() {
  return GetRequestContext();
}

net::URLRequestContextGetter*
CameoBrowserContext::GetMediaRequestContextForRenderProcess(
    int renderer_child_id) {
  return GetRequestContext();
}

net::URLRequestContextGetter*
CameoBrowserContext::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory) {
  return GetRequestContext();
}

content::ResourceContext* CameoBrowserContext::GetResourceContext() {
  return resource_context_.get();
}
