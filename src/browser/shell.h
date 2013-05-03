// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef CAMEO_SRC_BROWSER_SHELL_H_
#define CAMEO_SRC_BROWSER_SHELL_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/string_piece.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/web_contents_delegate.h"
#include "ipc/ipc_channel.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"

#if defined(TOOLKIT_GTK)
#include "ui/base/gtk/gtk_compat.h"
#include "ui/base/gtk/gtk_signal.h"

typedef struct _GtkToolItem GtkToolItem;
#elif defined(OS_ANDROID)
#include "base/android/scoped_java_ref.h"
#elif defined(USE_AURA)
#if defined(OS_CHROMEOS)
namespace content {
class MinimalAsh;
}
#endif
namespace views {
class Widget;
class ViewsDelegate;
}
#endif

class GURL;
namespace content {
class BrowserContext;
class SiteInstance;
class WebContents;
}

namespace cameo {

class NativeAppWindow;
class ShellDevToolsFrontend;
class ShellJavaScriptDialogManager;

// This represents one window of the Cameo Shell, i.e. all the UI including
// buttons and url bar, as well as the web content area.
class Shell : public content::WebContentsDelegate,
              public content::NotificationObserver {
 public:
  virtual ~Shell();

  void LoadURL(const GURL& url);
  void LoadURLForFrame(const GURL& url, const std::string& frame_name);
  void GoBackOrForward(int offset);
  void Reload();
  void Stop();
  void UpdateNavigationControls();
  void Close();
  void ShowDevTools();
  void CloseDevTools();

  bool is_devtools() const { return is_devtools_; }
  void set_is_devtools(bool is_devtools) { is_devtools_ = is_devtools; }

#if (defined(OS_WIN) && !defined(USE_AURA)) || defined(TOOLKIT_GTK)
  // Resizes the main window to the given dimensions.
  void SizeTo(int width, int height);
#endif

  // Do one time initialization at application startup.
  static void Initialize();

  static Shell* CreateNewWindow(content::BrowserContext* browser_context,
                                const GURL& url,
                                content::SiteInstance* site_instance,
                                int routing_id,
                                const gfx::Size& initial_size);

  // Returns the Shell object corresponding to the given RenderViewHost.
  static Shell* FromRenderViewHost(content::RenderViewHost* rvh);

  // Returns the currently open windows.
  static std::vector<Shell*>& windows() { return windows_; }

  // Closes all windows and returns. This runs a message loop.
  static void CloseAllWindows();

  // Closes all windows and exits.
  static void PlatformExit();

  // Used for content_browsertests. Called once.
  static void SetShellCreatedCallback(
      base::Callback<void(Shell* shell)> shell_created_callback);

  content::WebContents* web_contents() const { return web_contents_.get(); }

#if !defined(OS_WIN)
  gfx::NativeWindow window() { return window_; }
#endif

#if defined(OS_MACOSX)
  // Public to be called by an ObjC bridge object.
  void ActionPerformed(int control);
  void URLEntered(std::string url_string);
#elif defined(OS_ANDROID)
  // Registers the Android Java to native methods.
  static bool Register(JNIEnv* env);
#endif

  // WebContentsDelegate
  virtual content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;
  virtual void LoadingStateChanged(content::WebContents* source) OVERRIDE;
#if defined(OS_ANDROID)
  virtual void LoadProgressChanged(content::WebContents* source,
                                   double progress) OVERRIDE;
#endif
  virtual void ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                          bool enter_fullscreen) OVERRIDE;
  virtual bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) const OVERRIDE;
  virtual void RequestToLockMouse(content::WebContents* web_contents,
                                  bool user_gesture,
                                  bool last_unlocked_by_target) OVERRIDE;
  virtual void CloseContents(content::WebContents* source) OVERRIDE;
  virtual bool CanOverscrollContent() const OVERRIDE;
  virtual void WebContentsCreated(content::WebContents* source_contents,
                                  int64 source_frame_id,
                                  const string16& frame_name,
                                  const GURL& target_url,
                                  content::WebContents* new_contents) OVERRIDE;
  virtual void DidNavigateMainFramePostCommit(
      content::WebContents* web_contents) OVERRIDE;
  virtual content::JavaScriptDialogManager*
      GetJavaScriptDialogManager() OVERRIDE;
#if defined(OS_MACOSX)
  virtual void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) OVERRIDE;
#endif
  virtual void ActivateContents(content::WebContents* contents) OVERRIDE;
  virtual void DeactivateContents(content::WebContents* contents) OVERRIDE;

  bool headless() const { return headless_; }

 private:
  enum UIControl {
    BACK_BUTTON,
    FORWARD_BUTTON,
    STOP_BUTTON
  };

  explicit Shell(content::WebContents* web_contents);

  // Helper to create a new Shell given a newly created WebContents.
  static Shell* CreateShell(content::WebContents* web_contents);

  // Helper for one time initialization of application
  static void PlatformInitialize(const gfx::Size& default_window_size);

  // All the methods that begin with Platform need to be implemented by the
  // platform specific Shell implementation.
  // Called from the destructor to let each platform do any necessary cleanup.
  void PlatformCleanUp();
  // Creates the main window GUI.
  void PlatformCreateWindow(int width, int height);
  // Links the WebContents into the newly created window.
  void PlatformSetContents();
  // Resize the content area and GUI.
  void PlatformResizeSubViews();
  // Enable/disable a button.
  void PlatformEnableUIControl(UIControl control, bool is_enabled);
  // Updates the url in the url bar.
  void PlatformSetAddressBarURL(const GURL& url);
  // Sets whether the spinner is spinning.
  void PlatformSetIsLoading(bool loading);
  // Set the title of shell window
  void PlatformSetTitle(const string16& title);
#if defined(OS_ANDROID)
  void PlatformToggleFullscreenModeForTab(content::WebContents* web_contents,
                                          bool enter_fullscreen);
  bool PlatformIsFullscreenForTabOrPending(
      const content::WebContents* web_contents) const;
#endif

  gfx::NativeView GetContentView();

  // NotificationObserver
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

#if defined(OS_WIN) && !defined(USE_AURA)
  static ATOM RegisterWindowClass();
  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
  static LRESULT CALLBACK EditWndProc(HWND, UINT, WPARAM, LPARAM);
#elif defined(TOOLKIT_GTK)
  CHROMEGTK_CALLBACK_0(Shell, void, OnBackButtonClicked);
  CHROMEGTK_CALLBACK_0(Shell, void, OnForwardButtonClicked);
  CHROMEGTK_CALLBACK_0(Shell, void, OnReloadButtonClicked);
  CHROMEGTK_CALLBACK_0(Shell, void, OnStopButtonClicked);
  CHROMEGTK_CALLBACK_0(Shell, void, OnURLEntryActivate);
  CHROMEGTK_CALLBACK_0(Shell, gboolean, OnWindowDestroyed);

  CHROMEG_CALLBACK_3(Shell, gboolean, OnCloseWindowKeyPressed, GtkAccelGroup*,
                     GObject*, guint, GdkModifierType);
  CHROMEG_CALLBACK_3(Shell, gboolean, OnNewWindowKeyPressed, GtkAccelGroup*,
                     GObject*, guint, GdkModifierType);
  CHROMEG_CALLBACK_3(Shell, gboolean, OnHighlightURLView, GtkAccelGroup*,
                     GObject*, guint, GdkModifierType);
#endif

  scoped_ptr<ShellJavaScriptDialogManager> dialog_manager_;

  scoped_ptr<content::WebContents> web_contents_;

  ShellDevToolsFrontend* devtools_frontend_;

  bool is_fullscreen_;

  bool is_devtools_;

#if defined(OS_WIN)
  NativeAppWindow* window_;
#else
  // TODO(hmin): Decouple app window with Shell.
  gfx::NativeWindow window_;
#endif
  gfx::NativeEditView url_edit_view_;

  // Notification manager
  content::NotificationRegistrar registrar_;

#if defined(OS_WIN) && !defined(USE_AURA)
  WNDPROC default_edit_wnd_proc_;
  static HINSTANCE instance_handle_;
#elif defined(TOOLKIT_GTK)
  GtkWidget* vbox_;

  GtkToolItem* back_button_;
  GtkToolItem* forward_button_;
  GtkToolItem* reload_button_;
  GtkToolItem* stop_button_;

  GtkWidget* spinner_;
  GtkToolItem* spinner_item_;

  int content_width_;
  int content_height_;
#elif defined(OS_ANDROID)
  base::android::ScopedJavaGlobalRef<jobject> java_object_;
#elif defined(USE_AURA)
#if defined(OS_CHROMEOS)
  static content::MinimalAsh* minimal_ash_;
#endif
  static views::ViewsDelegate* views_delegate_;

  views::Widget* window_widget_;
#endif

  // Show chrome or not.
  bool headless_;

  // A container of all the open windows. We use a vector so we can keep track
  // of ordering.
  static std::vector<Shell*> windows_;

  static base::Callback<void(Shell* shell)> shell_created_callback_;

  // True if the destructur of Shell should post a quit closure on the current
  // message loop if the destructed Shell object was the last one.
  static bool quit_message_loop_;
};

}  // namespace content

#endif  // CAMEO_SRC_BROWSER_SHELL_H_
