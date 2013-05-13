#ifndef CAMEO_RENDERER_CAMEO_CONTENT_RENDERER_CLIENT_H_
#define CAMEO_RENDERER_CAMEO_CONTENT_RENDERER_CLIENT_H_

#include "content/public/renderer/content_renderer_client.h"

class CameoContentRendererClient : public content::ContentRendererClient {
 public:
  CameoContentRendererClient();
  virtual ~CameoContentRendererClient();

  virtual void RenderThreadStarted() OVERRIDE;
};

#endif  // CAMEO_RENDERER_CAMEO_CONTENT_RENDERER_CLIENT_H_
