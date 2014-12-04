// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_SPLASH_SCREEN_TIZEN_H_
#define XWALK_RUNTIME_BROWSER_UI_SPLASH_SCREEN_TIZEN_H_

#include "base/files/file_path.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/compositor/layer_animation_observer.h"

namespace content {
class WebContents;
class RenderViewHost;
}

namespace ui {
class Layer;
}

namespace views {
class Widget;
}

namespace xwalk {

class SplashScreenTizen : public content::WebContentsObserver,
                          public ui::ImplicitAnimationObserver {
 public:
  SplashScreenTizen(views::Widget* host,
                    const base::FilePath& file,
                    content::WebContents* web_contents);
  ~SplashScreenTizen();

  void Start();
  void Stop();

  // Overridden from content::WebContentsObserver.
  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override;

  void DidFailLoad(content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code,
                   const base::string16& error_description) override;


  // ui::ImplicitAnimationObserver overrides:
  void OnImplicitAnimationsCompleted() override;

 private:
  views::Widget* widget_host_;
  base::FilePath splash_screen_image_;

  scoped_ptr<ui::Layer> layer_;
  class SplashScreenLayerDelegate;
  scoped_ptr<SplashScreenLayerDelegate> layer_delegate_;

  bool is_started;

  DISALLOW_COPY_AND_ASSIGN(SplashScreenTizen);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_SPLASH_SCREEN_TIZEN_H_
