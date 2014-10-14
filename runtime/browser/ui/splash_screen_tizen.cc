// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/splash_screen_tizen.h"

#include "base/location.h"
#include "ui/compositor/layer.h"
#include "ui/compositor/scoped_layer_animation_settings.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/views/widget/widget.h"
#include "xwalk/runtime/browser/image_util.h"

namespace xwalk {

namespace {
const int kHideAnimationDuration = 1;  // second
}  // namespace

class SplashScreenTizen::SplashScreenLayerDelegate : public ui::LayerDelegate {
 public:
  SplashScreenLayerDelegate() {}

  virtual ~SplashScreenLayerDelegate() {}

  void set_image(const gfx::Image& image) { image_ = image; }
  const gfx::Image& image() const { return image_; }

  virtual void OnPaintLayer(gfx::Canvas* canvas) OVERRIDE {
    if (!image_.IsEmpty()) {
      canvas->DrawImageInt(image_.AsImageSkia(), 0, 0);
    } else {
      LOG(WARNING) << "The splash screen image is not loaded.";
    }
  }

  virtual void OnDelegatedFrameDamage(
      const gfx::Rect& damage_rect_in_dip) OVERRIDE {}

  virtual void OnDeviceScaleFactorChanged(float device_scale_factor) OVERRIDE {}

  virtual base::Closure PrepareForLayerBoundsChange() OVERRIDE {
    return base::Closure();
  }

 private:
  gfx::Image image_;

  DISALLOW_COPY_AND_ASSIGN(SplashScreenLayerDelegate);
};

SplashScreenTizen::SplashScreenTizen(views::Widget* host,
                                     const base::FilePath& file,
                                     content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      widget_host_(host),
      splash_screen_image_(file),
      layer_(new ui::Layer(ui::LAYER_TEXTURED)),
      layer_delegate_(new SplashScreenLayerDelegate()),
      is_started(false) {
  DCHECK(widget_host_);
  layer_->set_delegate(layer_delegate_.get());
}

SplashScreenTizen::~SplashScreenTizen() {}

void SplashScreenTizen::Start() {
  DCHECK(widget_host_);
  if (is_started)
    return;

  is_started = true;
  gfx::Image image = xwalk_utils::LoadImageFromFilePath(splash_screen_image_);
  if (!image.IsEmpty()) {
    layer_delegate_->set_image(image);
    ui::Layer* top_layer = widget_host_->GetLayer();
    gfx::Rect rc = gfx::Rect(widget_host_->GetWindowBoundsInScreen());
    // The bound of current layer locating at the host window.
    gfx::Rect layer_bound((rc.width() - image.Width()) / 2,
        (rc.height() - image.Height()) / 2, image.Width(), image.Height());
    layer_->SetBounds(layer_bound);
    top_layer->Add(layer_.get());
    top_layer->StackAtTop(layer_.get());
  }
}

void SplashScreenTizen::Stop() {
  DCHECK(widget_host_);
  if (!is_started)
    return;

  is_started = false;
  ui::ScopedLayerAnimationSettings settings(layer_->GetAnimator());
  settings.SetTransitionDuration(base::TimeDelta::FromSeconds(
      kHideAnimationDuration));
  settings.SetPreemptionStrategy(ui::LayerAnimator::REPLACE_QUEUED_ANIMATIONS);
  settings.AddObserver(this);
  layer_->SetOpacity(0.0f);
}

void SplashScreenTizen::DidFinishLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url) {
  Stop();
}

void SplashScreenTizen::DidFailLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    int error_code,
    const base::string16& error_description) {
  Stop();
}

void SplashScreenTizen::OnImplicitAnimationsCompleted() {
  DCHECK(widget_host_);
  widget_host_->GetLayer()->Remove(layer_.get());
}

}  // namespace xwalk
