// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/color_chooser.h"

#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/color_chooser.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "ui/views/color_chooser/color_chooser_listener.h"
#include "ui/views/color_chooser/color_chooser_view.h"
#include "ui/views/widget/widget.h"


class ColorChooserAura : public xwalk::ColorChooser,
                         public views::ColorChooserListener {
 public:
  static ColorChooserAura* Open(content::WebContents* web_contents,
                                SkColor initial_color);

  ColorChooserAura(content::WebContents* web_contents, SkColor initial_color);

 private:
  static ColorChooserAura* current_color_chooser_;

  // content::ColorChooser overrides:
  virtual void End() OVERRIDE;
  virtual void SetSelectedColor(SkColor color) OVERRIDE;

  // views::ColorChooserListener overrides:
  virtual void OnColorChosen(SkColor color) OVERRIDE;
  virtual void OnColorChooserDialogClosed() OVERRIDE;

  void DidEndColorChooser();

  // The actual view of the color chooser.
  // Ownership handled by parent.
  views::ColorChooserView* view_;

  // The widget for the color chooser.
  // Releases automatically when closed.
  views::Widget* widget_;

  // The web contents invoking the color chooser.
  // Outlives this class
  content::WebContents* web_contents_;

  DISALLOW_COPY_AND_ASSIGN(ColorChooserAura);
};

ColorChooserAura* ColorChooserAura::current_color_chooser_ = NULL;

ColorChooserAura::ColorChooserAura(content::WebContents* web_contents,
                                   SkColor initial_color)
    : web_contents_(web_contents) {
  view_ = new views::ColorChooserView(this, initial_color);
  widget_ = views::Widget::CreateWindowWithContext(
      view_, web_contents->GetView()->GetNativeView());
  widget_->SetAlwaysOnTop(true);
  widget_->Show();
  if (IsTesting()) {
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
        base::Bind(&ColorChooserAura::SetSelectedColor,
                   base::Unretained(this),
                   GetColorForBrowserTest()));
  }
}

void ColorChooserAura::OnColorChosen(SkColor color) {
  if (web_contents_)
    web_contents_->DidChooseColorInColorChooser(color);
  if (IsTesting()) {
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
        base::Bind(&ColorChooserAura::End, base::Unretained(this)));
  }
}

void ColorChooserAura::OnColorChooserDialogClosed() {
  view_ = NULL;
  widget_ = NULL;
  DidEndColorChooser();
}

void ColorChooserAura::End() {
  if (widget_ && widget_->IsVisible()) {
    view_->set_listener(NULL);
    widget_->Close();
    view_ = NULL;
    widget_ = NULL;
    DidEndColorChooser();
  }
}

void ColorChooserAura::DidEndColorChooser() {
  DCHECK(current_color_chooser_ == this);
  current_color_chooser_ = NULL;
  if (web_contents_)
    web_contents_->DidEndColorChooser();
}

void ColorChooserAura::SetSelectedColor(SkColor color) {
  if (view_) {
    view_->OnColorChanged(color);
    view_->OnSaturationValueChosen(view_->saturation(), view_->value());
  }
}

// static
ColorChooserAura* ColorChooserAura::Open(
    content::WebContents* web_contents, SkColor initial_color) {
  if (current_color_chooser_)
    current_color_chooser_->End();
  DCHECK(!current_color_chooser_);
  current_color_chooser_ = new ColorChooserAura(web_contents, initial_color);
  return current_color_chooser_;
}

namespace xwalk {

content::ColorChooser* ShowColorChooser(content::WebContents* web_contents,
                                        SkColor initial_color) {
  return ColorChooserAura::Open(web_contents, initial_color);
}

}  // namespace xwalk
