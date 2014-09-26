// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_ICON_HELPER_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_ICON_HELPER_H_

#include <string>
#include <vector>
#include "content/public/browser/web_contents_observer.h"
#include "url/gurl.h"

class SkBitmap;

namespace content {
struct FaviconURL;
class WebContents;
}

namespace gfx {
class Size;
}

namespace xwalk {

class XWalkIconHelper : public content::WebContentsObserver {
 public:
  class Listener {
   public:
    virtual void OnIconAvailable(const GURL& icon_url) = 0;
    virtual void OnReceivedIcon(const GURL& icon_url,
                                const SkBitmap& bitmap) = 0;
   protected:
    virtual ~Listener() {}
  };

  explicit XWalkIconHelper(content::WebContents* web_contents);
  virtual ~XWalkIconHelper();

  void SetListener(Listener* listener);

  void DownloadIcon(const GURL& icon_url);

  // From WebContentsObserver
  virtual void DidUpdateFaviconURL(
      const std::vector<content::FaviconURL>& candidates) OVERRIDE;

  void DownloadFaviconCallback(
      int id,
      int http_status_code,
      const GURL& image_url,
      const std::vector<SkBitmap>& bitmaps,
      const std::vector<gfx::Size>& original_bitmap_sizes);

 private:
  Listener* listener_;

  DISALLOW_COPY_AND_ASSIGN(XWalkIconHelper);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_ICON_HELPER_H_
