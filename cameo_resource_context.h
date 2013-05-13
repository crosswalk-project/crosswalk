#ifndef CAMEO_CAMEO_RESOURCE_CONTEXT_H_
#define CAMEO_CAMEO_RESOURCE_CONTEXT_H_

#include "content/public/browser/resource_context.h"

class CameoURLRequestContextGetter;

class CameoResourceContext : public content::ResourceContext {
 public:
  CameoResourceContext();
  virtual ~CameoResourceContext();

  // ResourceContext implementation.
  virtual net::HostResolver* GetHostResolver() OVERRIDE;
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE;

  void set_url_request_context_getter(CameoURLRequestContextGetter* getter) {
    getter_ = getter;
  }

 private:
  CameoURLRequestContextGetter* getter_;

  DISALLOW_COPY_AND_ASSIGN(CameoResourceContext);
};

#endif  // CAMEO_CAMEO_RESOURCE_CONTEXT_H_
