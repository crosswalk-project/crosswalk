#ifndef CAMEO_CAMEO_BROWSER_CONTEXT_H_
#define CAMEO_CAMEO_BROWSER_CONTEXT_H_

#include "content/public/browser/browser_context.h"

#include "content/public/browser/content_browser_client.h"

class CameoResourceContext;
class CameoURLRequestContextGetter;

class CameoBrowserContext : public content::BrowserContext {
 public:
  CameoBrowserContext();
  virtual ~CameoBrowserContext();

  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers);

  // BrowserContext implementation.
  virtual base::FilePath GetPath() OVERRIDE;

  virtual bool IsOffTheRecord() const { return false; }

  virtual net::URLRequestContextGetter* GetRequestContext();
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id);
  virtual net::URLRequestContextGetter* GetMediaRequestContext();
  virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id);

  virtual net::URLRequestContextGetter*
  GetMediaRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory);

  virtual content::ResourceContext* GetResourceContext();

  virtual content::DownloadManagerDelegate* GetDownloadManagerDelegate() {
    return NULL;
  }

  virtual content::GeolocationPermissionContext* GetGeolocationPermissionContext() {
    return NULL;
  }

  virtual content::SpeechRecognitionPreferences* GetSpeechRecognitionPreferences() {
    return NULL;
  }

  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() {
    return NULL;
  }

 private:
  scoped_ptr<CameoResourceContext> resource_context_;
  scoped_refptr<CameoURLRequestContextGetter> url_request_getter_;

  DISALLOW_COPY_AND_ASSIGN(CameoBrowserContext);
};

#endif  // CAMEO_CAMEO_BROWSER_CONTEXT_H_
