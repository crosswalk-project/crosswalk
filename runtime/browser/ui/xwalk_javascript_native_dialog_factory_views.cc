// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/xwalk_javascript_native_dialog_factory.h"

#include "base/memory/ptr_util.h"
#include "components/app_modal/javascript_dialog_manager.h"
#include "components/app_modal/javascript_native_dialog_factory.h"
#include "components/app_modal/views/javascript_app_modal_dialog_views.h"
#include "components/constrained_window/constrained_window_views.h"
#include "components/constrained_window/constrained_window_views_client.h"
#include "components/web_modal/web_contents_modal_dialog_manager.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"

#if defined(USE_AURA)
#include "ui/aura/window.h"
#endif

namespace {

class XWalkJavaScriptNativeDialogViewsFactory
    : public app_modal::JavaScriptNativeDialogFactory {
 public:
  XWalkJavaScriptNativeDialogViewsFactory() = default;
  ~XWalkJavaScriptNativeDialogViewsFactory() override = default;

 private:
  app_modal::NativeAppModalDialog* CreateNativeJavaScriptDialog(
      app_modal::JavaScriptAppModalDialog* dialog) override {
    app_modal::JavaScriptAppModalDialogViews* dialogViews =
        new app_modal::JavaScriptAppModalDialogViews(dialog);

    dialog->web_contents()->GetDelegate()->ActivateContents(
        dialog->web_contents());
    gfx::NativeWindow parent_window =
        dialog->web_contents()->GetTopLevelNativeWindow();
#if defined(USE_AURA)
    if (!parent_window->GetRootWindow()) {
      // When we are part of a WebContents that isn't actually being displayed
      // on the screen, we can't actually attach to it.
      parent_window = nullptr;
    }
#endif
    constrained_window::CreateBrowserModalDialogViews(dialogViews,
        parent_window);
    return dialogViews;
  }

  DISALLOW_COPY_AND_ASSIGN(XWalkJavaScriptNativeDialogViewsFactory);
};

class XWalkConstrainedWindowViewsClient
    : public constrained_window::ConstrainedWindowViewsClient {
 public:
  XWalkConstrainedWindowViewsClient() = default;
  ~XWalkConstrainedWindowViewsClient() override = default;

 private:
  // ConstrainedWindowViewsClient:
  web_modal::ModalDialogHost* GetModalDialogHost(
      gfx::NativeWindow parent) override {
    return nullptr;
  }

  gfx::NativeView GetDialogHostView(gfx::NativeWindow parent) override {
    return parent;
  }

  DISALLOW_COPY_AND_ASSIGN(XWalkConstrainedWindowViewsClient);
};

}  // namespace

void InstallXWalkJavaScriptNativeDialogFactory() {
  app_modal::JavaScriptDialogManager::GetInstance()->
      SetNativeDialogFactory(
          base::WrapUnique(new XWalkJavaScriptNativeDialogViewsFactory));

  constrained_window::SetConstrainedWindowViewsClient(
      base::WrapUnique(new XWalkConstrainedWindowViewsClient));
}

namespace web_modal {

SingleWebContentsDialogManager*
WebContentsModalDialogManager::CreateNativeWebModalManager(
gfx::NativeWindow dialog,
SingleWebContentsDialogManagerDelegate* native_delegate) {
  return nullptr;
}

}  // namespace web_modal
