#include "cameo/cameo_content_browser_client.h"

#include "cameo/cameo_browser_context.h"
#include "cameo/cameo_browser_main_parts.h"

CameoContentBrowserClient::CameoContentBrowserClient()
    : cameo_browser_main_parts_(NULL) {
}

content::BrowserMainParts* CameoContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams&) {
  cameo_browser_main_parts_ = new CameoBrowserMainParts();
  return cameo_browser_main_parts_;
}

net::URLRequestContextGetter* CameoContentBrowserClient::CreateRequestContext(
    content::BrowserContext* browser_context,
    content::ProtocolHandlerMap* protocol_handlers) {
  // FIXME: Check if browser_context is same as we have.
  return this->browser_context()->CreateRequestContext(protocol_handlers);
}

CameoBrowserContext* CameoContentBrowserClient::browser_context() {
  return cameo_browser_main_parts_->browser_context();
}
