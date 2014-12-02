// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_CONTENT_H_
#define XWALK_RUNTIME_BROWSER_XWALK_CONTENT_H_

#include <vector>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/favicon_url.h"
#include "url/gurl.h"
#include "ui/gfx/image/image.h"

namespace content {
class ColorChooser;
struct FileChooserParams;
class RenderProcessHost;
class SiteInstance;
class WebContents;
}

namespace xwalk {

class NativeAppWindow;
class XWalkBrowserContext;
class RuntimeUIDelegate;

// XWalkContent is responsible for maintaning its owned WebContents.
class XWalkContent : public content::WebContentsDelegate,
                     public content::WebContentsObserver,
                     public content::NotificationObserver {
 public:
  // New "contents" are also created from XWalkContent::WebContentsCreated which
  // is overridden WebContentsDelegate method. The "observer" is needed to
  // observe appearance of such XWalkContent instances.
  class Observer {
   public:
    // Called when a new XWalkContent instance is added.
    virtual void OnContentCreated(XWalkContent* content) = 0;

    // Called when a XWalkContent instance is closed.
    virtual void OnContentClosed(XWalkContent* content) = 0;

   protected:
      virtual ~Observer() {}
  };

  // Fullscreen options.
  enum FullscreenOptions {
    NO_FULLSCREEN = 0,
    // Fullscreen entered by launch with "--fullscreen".
    FULLSCREEN_FOR_LAUNCH = 1,
    // Fullscreen entered by HTML requestFullscreen.
    FULLSCREEN_FOR_TAB = 1 << 1,
  };
  virtual ~XWalkContent();

  void set_ui_delegate(RuntimeUIDelegate* ui_delegate) {
    ui_delegate_ = ui_delegate;
  }
  void set_observer(Observer* observer) { observer_ = observer; }

  // Create a new XWalkContent instance with the given browsing context.
  static XWalkContent* Create(XWalkBrowserContext* context,
                         content::SiteInstance* site = nullptr);

  void LoadURL(const GURL& url);
  void Show();
  void Close();

  content::WebContents* web_contents() const { return web_contents_.get(); }
  // FIXME : This method should be removed.
  NativeAppWindow* window();
  gfx::Image app_icon() const { return app_icon_; }
  unsigned int fullscreen_options() { return fullscreen_options_; }
  void set_fullscreen_options(unsigned int options) {
    fullscreen_options_ = options;
  }

  content::RenderProcessHost* GetRenderProcessHost();

  void set_remote_debugging_enabled(bool enable) {
    remote_debugging_enabled_ = enable;
  }
  bool remote_debugging_enabled() const { return remote_debugging_enabled_; }

 protected:
  explicit XWalkContent(content::WebContents* web_contents);

    // Overridden from content::WebContentsDelegate:
  virtual content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;
  virtual void LoadingStateChanged(content::WebContents* source,
                                   bool to_different_document) OVERRIDE;
  virtual void ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                          bool enter_fullscreen) OVERRIDE;
  virtual bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) const OVERRIDE;
  virtual void RequestToLockMouse(content::WebContents* web_contents,
                                  bool user_gesture,
                                  bool last_unlocked_by_target) OVERRIDE;
  virtual void CloseContents(content::WebContents* source) OVERRIDE;
  virtual void WebContentsCreated(content::WebContents* source_contents,
                                  int opener_render_frame_id,
                                  const base::string16& frame_name,
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
      SkColor initial_color,
      const std::vector<content::ColorSuggestion>& suggestions) OVERRIDE;
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
  virtual void DidUpdateFaviconURL(
      const std::vector<content::FaviconURL>& candidates) OVERRIDE;

  // Callback method for WebContents::DownloadImage.
  void DidDownloadFavicon(int id,
                          int http_status_code,
                          const GURL& image_url,
                          const std::vector<SkBitmap>& bitmaps,
                          const std::vector<gfx::Size>& sizes);

  // NotificationObserver
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  // Notification manager.
  content::NotificationRegistrar registrar_;

  // The WebContents owned by this runtime.
  scoped_ptr<content::WebContents> web_contents_;

  gfx::Image app_icon_;

  unsigned int fullscreen_options_;
  bool remote_debugging_enabled_;
  RuntimeUIDelegate* ui_delegate_;
  Observer* observer_;
  base::WeakPtrFactory<XWalkContent> weak_ptr_factory_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_CONTENT_H_
