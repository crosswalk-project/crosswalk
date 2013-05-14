// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_RUNTIME_H_
#define CAMEO_SRC_RUNTIME_BROWSER_RUNTIME_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "cameo/src/runtime/browser/ui/native_app_window.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "googleurl/src/gurl.h"
#include "ui/gfx/image/image.h"

namespace content {
class WebContents;
}

namespace cameo {

class NativeAppWindow;
class RuntimeContext;

// Runtime represents the running environment for a web page. It is responsible
// for maintaning its owned WebContents and handling any communication between
// WebContents and native app window.
class Runtime : public content::WebContentsDelegate,
                public content::NotificationObserver {
 public:
  // Create a new Runtime instance with the given browsing context.
  static Runtime* Create(RuntimeContext* runtime_context, const GURL& url);
  // Create a new Runtime instance for the given web contents.
  static Runtime* CreateFromWebContents(content::WebContents* web_contents);

  void LoadURL(const GURL& url);
  void Close();

  content::WebContents* web_contents() const { return web_contents_.get(); }
  NativeAppWindow* window() const;
  RuntimeContext* runtime_context() const { return runtime_context_; }
  gfx::Image app_icon() const;

 protected:
  explicit Runtime(RuntimeContext* runtime_context);
  explicit Runtime(content::WebContents* web_contents);
  virtual ~Runtime();

  // Initialize the app window.
  void InitAppWindow(const NativeAppWindow::CreateParams& params);

  // Overridden from content::WebContentsDelegate:
  virtual content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;
  virtual void LoadingStateChanged(content::WebContents* source) OVERRIDE;
  virtual void ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                          bool enter_fullscreen) OVERRIDE;
  virtual bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) const OVERRIDE;
  virtual void RequestToLockMouse(content::WebContents* web_contents,
                                  bool user_gesture,
                                  bool last_unlocked_by_target) OVERRIDE;
  virtual void CloseContents(content::WebContents* source) OVERRIDE;
  virtual void WebContentsCreated(content::WebContents* source_contents,
                                  int64 source_frame_id,
                                  const string16& frame_name,
                                  const GURL& target_url,
                                  content::WebContents* new_contents) OVERRIDE;
  virtual void DidNavigateMainFramePostCommit(
      content::WebContents* web_contents) OVERRIDE;
  virtual content::JavaScriptDialogManager*
      GetJavaScriptDialogManager() OVERRIDE;
  virtual void ActivateContents(content::WebContents* contents) OVERRIDE;
  virtual void DeactivateContents(content::WebContents* contents) OVERRIDE;
  virtual bool CanOverscrollContent() const OVERRIDE;
  virtual bool PreHandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event,
      bool* is_keyboard_shortcut) OVERRIDE;
  virtual void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) OVERRIDE;

  // NotificationObserver
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  // The browsing context.
  cameo::RuntimeContext* runtime_context_;

  // Notification manager.
  content::NotificationRegistrar registrar_;

  // The WebContents owned by this runtime.
  scoped_ptr<content::WebContents> web_contents_;

  NativeAppWindow* window_;
};

}  // namespace cameo

#endif  // CAMEO_SRC_RUNTIME_BROWSER_RUNTIME_H_
