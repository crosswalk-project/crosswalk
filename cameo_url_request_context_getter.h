#ifndef CAMEO_CAMEO_URL_REQUEST_CONTEXT_GETTER_H_
#define CAMEO_CAMEO_URL_REQUEST_CONTEXT_GETTER_H_

#include "content/public/browser/content_browser_client.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_context_storage.h"

class CameoURLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  CameoURLRequestContextGetter(const base::FilePath& base_path,
                               content::ProtocolHandlerMap* protocol_handlers);
  virtual ~CameoURLRequestContextGetter();

  // net::URLRequestContextGetter implementation.
  virtual net::URLRequestContext* GetURLRequestContext() OVERRIDE;
  virtual scoped_refptr<base::SingleThreadTaskRunner>
      GetNetworkTaskRunner() const OVERRIDE;

  net::HostResolver* host_resolver();

 private:
  base::FilePath base_path_;

  scoped_ptr<net::URLRequestContext> url_request_context_;
  scoped_ptr<net::NetworkDelegate> network_delegate_;
  scoped_ptr<net::URLRequestContextStorage> storage_;
  content::ProtocolHandlerMap protocol_handlers_;
};

#endif  // CAMEO_CAMEO_URL_REQUEST_CONTEXT_GETTER_H_
