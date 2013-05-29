// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/ui/gtk/gtk_util.h"

#include <algorithm>
#include <map>

#include "base/environment.h"
#include "base/i18n/rtl.h"
#include "base/logging.h"
#include "base/nix/xdg_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/utf_string_conversions.h"
#include "ui/base/gtk/gtk_compat.h"
#include "ui/base/gtk/gtk_hig_constants.h"
#include "ui/base/gtk/gtk_screen_util.h"
#include "ui/base/gtk/menu_label_accelerator_util.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/text/text_elider.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/pango_util.h"

// These conflict with base/tracked_objects.h, so need to come last.
#include <gdk/gdkx.h>  // NOLINT

namespace gtk_util {

void GetWidgetSizeFromCharacters(
    GtkWidget* widget, double width_chars, double height_lines,
    int* width, int* height) {
  DCHECK(gtk_widget_get_realized(widget))
      << " widget must be realized to compute font metrics correctly";
  PangoContext* context = gtk_widget_create_pango_context(widget);
  GtkStyle* style = gtk_widget_get_style(widget);
  PangoFontMetrics* metrics = pango_context_get_metrics(context,
      style->font_desc, pango_context_get_language(context));
  if (width) {
    *width = static_cast<int>(
        pango_font_metrics_get_approximate_char_width(metrics) *
        width_chars / PANGO_SCALE);
  }
  if (height) {
    *height = static_cast<int>(
        (pango_font_metrics_get_ascent(metrics) +
        pango_font_metrics_get_descent(metrics)) *
        height_lines / PANGO_SCALE);
  }
  pango_font_metrics_unref(metrics);
  g_object_unref(context);
}

void GetWidgetSizeFromResources(
    GtkWidget* widget, int width_chars, int height_lines,
    int* width, int* height) {
  DCHECK(gtk_widget_get_realized(widget))
      << " widget must be realized to compute font metrics correctly";

  double chars = 0;
  if (width)
    base::StringToDouble(l10n_util::GetStringUTF8(width_chars), &chars);

  double lines = 0;
  if (height)
    base::StringToDouble(l10n_util::GetStringUTF8(height_lines), &lines);

  GetWidgetSizeFromCharacters(widget, chars, lines, width, height);
}

void ShowDialogWithMinLocalizedWidth(GtkWidget* dialog,
                                     int width_id) {
  gtk_widget_show_all(dialog);

  // Suggest a minimum size.
  gint width;
  GtkRequisition req;
  gtk_widget_size_request(dialog, &req);
  gtk_util::GetWidgetSizeFromResources(dialog, width_id, 0, &width, NULL);
  if (width > req.width)
    gtk_widget_set_size_request(dialog, width, -1);
}

void ApplyMessageDialogQuirks(GtkWidget* dialog) {
  if (gtk_window_get_modal(GTK_WINDOW(dialog))) {
    // Work around a KDE 3 window manager bug.
    scoped_ptr<base::Environment> env(base::Environment::Create());
    if (base::nix::DESKTOP_ENVIRONMENT_KDE3 ==
        base::nix::GetDesktopEnvironment(env.get()))
      gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), FALSE);
  }
}

}  // namespace gtk_util
