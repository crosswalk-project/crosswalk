#include "cameo/cameo_web_contents_delegate.h"

#include "content/public/browser/web_contents.h"
#include <iostream>

CameoWebContentsDelegate::CameoWebContentsDelegate() {
}

void CameoWebContentsDelegate::LoadingStateChanged(
    content::WebContents* source) {
  std::cout << "URL = " << source->GetURL() << "\n"
            << "IsLoading = " << source->IsLoading() << "\n";
}
