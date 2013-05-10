#include "cameo/cameo_resource_context.h"

#include "base/files/file_path.h"
#include "cameo/cameo_url_request_context_getter.h"

CameoResourceContext::CameoResourceContext()
    : getter_(NULL) {
}

CameoResourceContext::~CameoResourceContext() {
}

net::HostResolver* CameoResourceContext::GetHostResolver() {
  CHECK(getter_);
  return getter_->host_resolver();
}

net::URLRequestContext* CameoResourceContext::GetRequestContext() {
  CHECK(getter_);
  return getter_->GetURLRequestContext();
}
