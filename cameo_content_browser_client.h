#ifndef CAMEO_CAMEO_CONTENT_BROWSER_CLIENT_H_
#define CAMEO_CAMEO_CONTENT_BROWSER_CLIENT_H_

#include "content/public/browser/content_browser_client.h"

class CameoBrowserContext;
class CameoBrowserMainParts;

class CameoContentBrowserClient : public content::ContentBrowserClient {
 public:
  CameoContentBrowserClient();

  virtual content::BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams&);

  virtual net::URLRequestContextGetter* CreateRequestContext(
      content::BrowserContext* browser_context,
      content::ProtocolHandlerMap* protocol_handlers) OVERRIDE;

  CameoBrowserContext* browser_context();

 private:
  CameoBrowserMainParts* cameo_browser_main_parts_;
};

#endif  // CAMEO_CAMEO_CONTENT_BROWSER_CLIENT_H_
