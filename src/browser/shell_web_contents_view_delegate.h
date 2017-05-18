// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_H_
#define CAMEO_SRC_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_H_

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/context_menu_params.h"
#include "content/public/common/context_menu_source_type.h"

#if defined(TOOLKIT_GTK)
#include "ui/base/gtk/gtk_signal.h"
#include "ui/base/gtk/owned_widget_gtk.h"
#endif

namespace cameo {

class ShellWebContentsViewDelegate : public content::WebContentsViewDelegate {
 public:
  explicit ShellWebContentsViewDelegate(content::WebContents* web_contents);
  virtual ~ShellWebContentsViewDelegate();

  // Overridden from WebContentsViewDelegate:
  virtual void ShowContextMenu(const content::ContextMenuParams& params,
                               content::ContextMenuSourceType type) OVERRIDE;
  virtual content::WebDragDestDelegate* GetDragDestDelegate() OVERRIDE;

#if defined(TOOLKIT_GTK)
  virtual void Initialize(GtkWidget* expanded_container,
                          ui::FocusStoreGtk* focus_store) OVERRIDE;
  virtual gfx::NativeView GetNativeView() const OVERRIDE;
  virtual void Focus() OVERRIDE;
  virtual gboolean OnNativeViewFocusEvent(GtkWidget* widget,
                                          GtkDirectionType type,
                                          gboolean* return_value) OVERRIDE;
#elif defined(OS_MACOSX)
  virtual NSObject<RenderWidgetHostViewMacDelegate>*
      CreateRenderWidgetHostViewDelegate(
          RenderWidgetHost* render_widget_host) OVERRIDE;
  void ActionPerformed(int id);
#elif defined(OS_WIN)
  virtual void StoreFocus() OVERRIDE;
  virtual void RestoreFocus() OVERRIDE;
  virtual bool Focus() OVERRIDE;
  virtual void TakeFocus(bool reverse) OVERRIDE;
  virtual void SizeChanged(const gfx::Size& size) OVERRIDE;
  void MenuItemSelected(int selection);
#endif

 private:
  content::WebContents* web_contents_;
  content::ContextMenuParams params_;

#if defined(TOOLKIT_GTK)
  ui::OwnedWidgetGtk floating_;
  GtkWidget* expanded_container_;

  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnBackMenuActivated);
  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnForwardMenuActivated);
  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnReloadMenuActivated);
  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnOpenURLMenuActivated);
  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnCutMenuActivated);
  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnCopyMenuActivated);
  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnPasteMenuActivated);
  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnDeleteMenuActivated);
  CHROMEGTK_CALLBACK_0(ShellWebContentsViewDelegate, void,
                       OnInspectMenuActivated);
#endif

  DISALLOW_COPY_AND_ASSIGN(ShellWebContentsViewDelegate);
};

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_H_
