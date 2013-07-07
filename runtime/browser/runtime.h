// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_H_

#include <vector>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/favicon_url.h"
#include "googleurl/src/gurl.h"
#include "ui/gfx/image/image.h"

namespace content {
class ColorChooser;
struct FileChooserParams;
class WebContents;
}

namespace xwalk {

class NativeAppWindow;
class RuntimeContext;

// Runtime represents the running environment for a web page. It is responsible
// for maintaning its owned WebContents and handling any communication between
// WebContents and native app window.
class Runtime : public content::WebContentsDelegate,
                public content::WebContentsObserver,
                public content::NotificationObserver,
                public NativeAppWindowDelegate {
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
  gfx::Image app_icon() const { return app_icon_; }

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
  virtual content::ColorChooser* OpenColorChooser(
      content::WebContents* web_contents,
      int color_chooser_id,
      SkColor color) OVERRIDE;
  virtual void DidEndColorChooser() OVERRIDE;
  virtual void RunFileChooser(
      content::WebContents* web_contents,
      const content::FileChooserParams& params) OVERRIDE;
  virtual void EnumerateDirectory(content::WebContents* web_contents,
                                  int request_id,
                                  const base::FilePath& path) OVERRIDE;
  virtual void RequestMediaAccessPermission(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback) OVERRIDE;

  // Overridden from content::WebContentsObserver.
  virtual void DidUpdateFaviconURL(int32 page_id,
      const std::vector<content::FaviconURL>& candidates) OVERRIDE;

  // Callback method for WebContents::DownloadImage.
  void DidDownloadFavicon(int id,
                          const GURL& image_url,
                          int requested_size,
                          const std::vector<SkBitmap>& bitmaps);

  // NotificationObserver
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  // NativeAppWindowDelegate implementation.
  virtual void OnWindowDestroyed() OVERRIDE;

  // The browsing context.
  xwalk::RuntimeContext* runtime_context_;

  // Notification manager.
  content::NotificationRegistrar registrar_;

  // The WebContents owned by this runtime.
  scoped_ptr<content::WebContents> web_contents_;

  NativeAppWindow* window_;

  gfx::Image app_icon_;

  base::WeakPtrFactory<Runtime> weak_ptr_factory_;

  // Currently open color chooser. Non-NULL after OpenColorChooser is called and
  // before DidEndColorChooser is called.
  scoped_ptr<content::ColorChooser> color_chooser_;

  // Fullscreen options.
  enum FullscreenOptions {
    NO_FULLSCREEN = 0,
    // Fullscreen entered by launch with "--fullscreen".
    FULLSCREEN_FOR_LAUNCH = 1,
    // Fullscreen entered by HTML requestFullscreen.
    FULLSCREEN_FOR_TAB = 1 << 1,
  };

  unsigned int fullscreen_options_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_H_
