#ifndef CAMEO_CAMEO_WEB_CONTENTS_DELEGATE_H_
#define CAMEO_CAMEO_WEB_CONTENTS_DELEGATE_H_

#include "content/public/browser/web_contents_delegate.h"

class CameoWebContentsDelegate : public content::WebContentsDelegate {
 public:
  CameoWebContentsDelegate();

  virtual void LoadingStateChanged(content::WebContents* source) OVERRIDE;
};

#endif  // CAMEO_CAMEO_WEB_CONTENTS_DELEGATE_H_
