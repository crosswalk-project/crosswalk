// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "xwalk/runtime/browser/runtime_ui_delegate.h"
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

class XWalkAutofillManager;
class XWalkBrowserContext;
class RuntimeUIDelegate;

// Runtime represents the running environment for a web page. It is responsible
// for maintaning its owned WebContents.
class Runtime : public content::WebContentsDelegate,
                public content::WebContentsObserver,
                public content::NotificationObserver {
 public:
  // New "Runtimes" are also created from Runtime::WebContentsCreated which
  // is overridden WebContentsDelegate method. The "observer" is needed to
  // observe appearance and removal of such Runtime instances.
  class Observer {
   public:
      // Called when a new Runtime instance is added.
      virtual void OnNewRuntimeAdded(Runtime* new_runtime) = 0;

      // Called when a Runtime instance is removed.
      virtual void OnRuntimeClosed(Runtime* runtime) = 0;

      // Called when there is a request to exit the application.
      virtual void OnApplicationExitRequested(Runtime* runtime) = 0;

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
  ~Runtime() override;

  void set_ui_delegate(RuntimeUIDelegate* ui_delegate) {
    ui_delegate_ = ui_delegate;
  }
  void set_observer(Observer* observer) { observer_ = observer; }

  // Create a new Runtime instance with the given browsing context.
  static Runtime* Create(XWalkBrowserContext* context,
                         scoped_refptr<content::SiteInstance> site = nullptr);

  void LoadURL(const GURL& url);
  void Show();
  void Close();

  // Navigation methods.
  void Back();
  void Forward();
  void Reload();
  void Stop();

  content::WebContents* web_contents() const { return web_contents_.get(); }
  // FIXME : This method should be removed.
  NativeAppWindow* window();
  gfx::Image app_icon() const { return app_icon_; }
  unsigned int fullscreen_options() { return fullscreen_options_; }
  void set_fullscreen_options(unsigned int options) {
    fullscreen_options_ = options;
  }

  content::RenderProcessHost* GetRenderProcessHost();
  bool AddDownloadItem(content::DownloadItem* download_item,
      const content::DownloadTargetCallback& callback,
      const base::FilePath& suggested_path);
  void RequestApplicationExit();

 protected:
  explicit Runtime(content::WebContents* web_contents);

    // Overridden from content::WebContentsDelegate:
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) override;
  void LoadingStateChanged(content::WebContents* source,
                           bool to_different_document) override;
  void EnterFullscreenModeForTab(content::WebContents* web_contents,
                                 const GURL& origin) override;
  void ExitFullscreenModeForTab(
      content::WebContents* web_contents) override;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) const override;
  blink::WebDisplayMode GetDisplayMode(
      const content::WebContents* web_contents) const override;
  void RequestToLockMouse(content::WebContents* web_contents,
                          bool user_gesture,
                          bool last_unlocked_by_target) override;
  void CloseContents(content::WebContents* source) override;
  void WebContentsCreated(content::WebContents* source_contents,
                          int opener_render_frame_id,
                          const std::string& frame_name,
                          const GURL& target_url,
                          content::WebContents* new_contents) override;
  void DidNavigateMainFramePostCommit(
      content::WebContents* web_contents) override;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager(
      content::WebContents* contents) override;
  void ActivateContents(content::WebContents* contents) override;
  bool CanOverscrollContent() const override;
  bool PreHandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event,
      bool* is_keyboard_shortcut) override;
  void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) override;
  content::ColorChooser* OpenColorChooser(
      content::WebContents* web_contents,
      SkColor initial_color,
      const std::vector<content::ColorSuggestion>& suggestions) override;
  void RunFileChooser(
      content::RenderFrameHost* render_frame_host,
      const content::FileChooserParams& params) override;
  void EnumerateDirectory(content::WebContents* web_contents,
                          int request_id,
                          const base::FilePath& path) override;
  void RequestMediaAccessPermission(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback) override;
  bool CheckMediaAccessPermission(content::WebContents* web_contents,
                                  const GURL& security_origin,
                                  content::MediaStreamType type) override;
  void LoadProgressChanged(content::WebContents* source,
                           double progress) override;

  // Overridden from content::WebContentsObserver.
  void DidUpdateFaviconURL(
      const std::vector<content::FaviconURL>& candidates) override;
  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) override;
  void DidNavigateAnyFrame(
      content::RenderFrameHost* render_frame_host,
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override;

  // Callback method for WebContents::DownloadImage.
  void DidDownloadFavicon(int id,
                          int http_status_code,
                          const GURL& image_url,
                          const std::vector<SkBitmap>& bitmaps,
                          const std::vector<gfx::Size>& sizes);
  bool HandleContextMenu(const content::ContextMenuParams& params) override;

  // NotificationObserver
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // Notification manager.
  content::NotificationRegistrar registrar_;
  base::ThreadChecker thread_checker_;

  std::unique_ptr<content::WebContents> web_contents_;
#if !defined(OS_ANDROID)
  std::unique_ptr<XWalkAutofillManager> xwalk_autofill_manager_;
#endif

  gfx::Image app_icon_;

  unsigned int fullscreen_options_;
  RuntimeUIDelegate* ui_delegate_;
  Observer* observer_;
  base::WeakPtrFactory<Runtime> weak_ptr_factory_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_H_
