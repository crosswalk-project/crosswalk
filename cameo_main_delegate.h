#ifndef CAMEO_CAMEO_MAIN_DELEGATE_H_
#define CAMEO_CAMEO_MAIN_DELEGATE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/app/content_main_delegate.h"

class CameoMainDelegate : public content::ContentMainDelegate {
 public:
  CameoMainDelegate() {}

  // ContentMainDelegate implementation.
  virtual void PreSandboxStartup() OVERRIDE;
  virtual content::ContentBrowserClient* CreateContentBrowserClient() OVERRIDE;
  virtual content::ContentRendererClient* CreateContentRendererClient() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(CameoMainDelegate);
};

#endif  // CAMEO_CAMEO_MAIN_DELEGATE_H_
