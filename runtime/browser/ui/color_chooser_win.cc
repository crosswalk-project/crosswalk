// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include "xwalk/runtime/browser/runtime_platform_util.h"
#include "xwalk/runtime/browser/ui/color_chooser.h"
#include "xwalk/runtime/browser/ui/color_chooser_dialog_win.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/views/color_chooser/color_chooser_listener.h"

class ColorChooserWin : public xwalk::ColorChooser,
                        public views::ColorChooserListener {
 public:
  ColorChooserWin(int identifier,
                  content::WebContents* tab,
                  SkColor initial_color);
  ~ColorChooserWin();

  // content::ColorChooser overrides:
  virtual void End() OVERRIDE {}
  virtual void SetSelectedColor(SkColor color) OVERRIDE {}

  // views::ColorChooserListener overrides:
  virtual void OnColorChosen(SkColor color);
  virtual void OnColorChooserDialogClosed();

 private:
  // The web contents invoking the color chooser.  No ownership. because it will
  // outlive this class.
  content::WebContents* tab_;

  // The color chooser dialog which maintains the native color chooser UI.
  scoped_refptr<ColorChooserDialog> color_chooser_dialog_;
};

content::ColorChooser* content::ColorChooser::Create(int identifier,
                                                     content::WebContents* tab,
                                                     SkColor initial_color) {
  return new ColorChooserWin(identifier, tab, initial_color);
}

ColorChooserWin::ColorChooserWin(int identifier,
                                 content::WebContents* tab,
                                 SkColor initial_color)
    : xwalk::ColorChooser(identifier),
      tab_(tab) {
  gfx::NativeWindow owning_window = platform_util::GetTopLevel(
      tab_->GetRenderViewHost()->GetView()->GetNativeView());
  color_chooser_dialog_ = new ColorChooserDialog(this,
                                                 initial_color,
                                                 owning_window);
}

ColorChooserWin::~ColorChooserWin() {
  // Always call End() before destroying.
  DCHECK(!color_chooser_dialog_);
}

void ColorChooserWin::OnColorChosen(SkColor color) {
  if (tab_)
    tab_->DidChooseColorInColorChooser(identifier(), color);
}

void ColorChooserWin::OnColorChooserDialogClosed() {
  if (color_chooser_dialog_.get()) {
    color_chooser_dialog_->ListenerDestroyed();
    color_chooser_dialog_ = NULL;
  }
  if (tab_)
    tab_->DidEndColorChooser(identifier());
}

