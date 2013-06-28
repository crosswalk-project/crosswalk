// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtk/gtk.h>

#include "cameo/runtime/browser/ui/color_chooser.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "grit/cameo_resources.h"
#include "ui/base/gtk/gtk_signal.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/skia_utils_gtk.h"

namespace {

gboolean ClickButtonForTest(GtkWidget* widget) {
  g_signal_emit_by_name(widget, "clicked");
  return FALSE;
}

}  // namespace

class ColorChooserGtk : public cameo::ColorChooser,
                        public content::WebContentsObserver {
 public:
  ColorChooserGtk(
      int identifier, content::WebContents* tab, SkColor initial_color);
  virtual ~ColorChooserGtk();

  virtual void End() OVERRIDE;
  virtual void SetSelectedColor(SkColor color) OVERRIDE;

 private:
  CHROMEGTK_CALLBACK_0(ColorChooserGtk, void, OnColorChooserOk);
  CHROMEGTK_CALLBACK_0(ColorChooserGtk, void, OnColorChooserCancel);
  CHROMEGTK_CALLBACK_0(ColorChooserGtk, void, OnColorChooserDestroy);

  GtkWidget* color_selection_dialog_;
};

content::ColorChooser* content::ColorChooser::Create(
    int identifier, content::WebContents* tab, SkColor initial_color) {
  return new ColorChooserGtk(identifier, tab, initial_color);
}

ColorChooserGtk::ColorChooserGtk(
    int identifier, content::WebContents* tab, SkColor initial_color)
    : cameo::ColorChooser(identifier),
      content::WebContentsObserver(tab) {
  color_selection_dialog_ = gtk_color_selection_dialog_new(
      l10n_util::GetStringUTF8(IDS_SELECT_COLOR_DIALOG_TITLE).c_str());
  GtkWidget* cancel_button;
  GtkColorSelection* color_selection;
  GtkWidget* ok_button;
  g_object_get(color_selection_dialog_,
               "cancel-button", &cancel_button,
               "color-selection", &color_selection,
               "ok-button", &ok_button,
               NULL);
  gtk_color_selection_set_has_opacity_control(color_selection, FALSE);
  g_signal_connect(ok_button, "clicked",
                   G_CALLBACK(OnColorChooserOkThunk), this);
  g_signal_connect(cancel_button, "clicked",
                   G_CALLBACK(OnColorChooserCancelThunk), this);
  g_signal_connect(color_selection_dialog_, "destroy",
                   G_CALLBACK(OnColorChooserDestroyThunk), this);
  GdkColor gdk_color = gfx::SkColorToGdkColor(initial_color);
  gtk_color_selection_set_previous_color(color_selection, &gdk_color);
  gtk_color_selection_set_current_color(color_selection, &gdk_color);
  gtk_window_present(GTK_WINDOW(color_selection_dialog_));
  g_object_unref(cancel_button);
  g_object_unref(color_selection);
  g_object_unref(ok_button);
  if (IsTesting()) {
    SetSelectedColor(GetColorForBrowserTest());
    g_timeout_add(100, (GSourceFunc) ClickButtonForTest, ok_button);
  }
}

ColorChooserGtk::~ColorChooserGtk() {
  // Always call End() before destroying.
  DCHECK(!color_selection_dialog_);
}

void ColorChooserGtk::OnColorChooserOk(GtkWidget* widget) {
  GdkColor color;
  GtkColorSelection* color_selection;
  g_object_get(color_selection_dialog_,
               "color-selection", &color_selection, NULL);
  gtk_color_selection_get_current_color(color_selection, &color);
  web_contents()->DidChooseColorInColorChooser(identifier(),
                                               gfx::GdkColorToSkColor(color));
  g_object_unref(color_selection);
  gtk_widget_destroy(color_selection_dialog_);
}

void ColorChooserGtk::OnColorChooserCancel(GtkWidget* widget) {
  gtk_widget_destroy(color_selection_dialog_);
}

void ColorChooserGtk::OnColorChooserDestroy(GtkWidget* widget) {
  color_selection_dialog_ = NULL;
  if (web_contents())
    web_contents()->DidEndColorChooser(identifier());
}

void ColorChooserGtk::End() {
  if (!color_selection_dialog_)
    return;

  gtk_widget_destroy(color_selection_dialog_);
}

void ColorChooserGtk::SetSelectedColor(SkColor color) {
  if (!color_selection_dialog_)
    return;

  GdkColor gdk_color = gfx::SkColorToGdkColor(color);
  GtkColorSelection* color_selection;
  g_object_get(color_selection_dialog_,
               "color-selection", &color_selection, NULL);
  gtk_color_selection_set_previous_color(color_selection, &gdk_color);
  gtk_color_selection_set_current_color(color_selection, &gdk_color);
  g_object_unref(color_selection);
}
