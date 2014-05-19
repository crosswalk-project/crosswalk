// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "testing/gmock/include/gmock/gmock.h"

#if defined(USE_AURA)
#include "ui/aura/window.h"
#endif

using xwalk::NativeAppWindow;
using xwalk::Runtime;
using content::WebContents;
using testing::_;

// Observer for NOTIFICATION_FULLSCREEN_CHANGED notifications.
class FullscreenNotificationObserver
    : public content::WindowedNotificationObserver {
 public:
  FullscreenNotificationObserver() : WindowedNotificationObserver(
      xwalk::NOTIFICATION_FULLSCREEN_CHANGED,
      content::NotificationService::AllSources()) {}
 private:
  DISALLOW_COPY_AND_ASSIGN(FullscreenNotificationObserver);
};

class XWalkRuntimeTest : public InProcessBrowserTest {
 public:
  XWalkRuntimeTest() {}
  virtual ~XWalkRuntimeTest() {
    original_runtimes_.clear();
    notification_observer_.reset();
  }

  void Relaunch(const CommandLine& new_command_line) {
    base::LaunchProcess(new_command_line, base::LaunchOptions(), NULL);
  }

  // SetUpOnMainThread is called after BrowserMainRunner was initialized and
  // just before RunTestOnMainThread (aka. TestBody).
  virtual void SetUpOnMainThread() OVERRIDE {
    notification_observer_.reset(
        new content::WindowedNotificationObserver(
          xwalk::NOTIFICATION_RUNTIME_OPENED,
          content::NotificationService::AllSources()));
    original_runtimes_.assign(runtimes().begin(), runtimes().end());
  }

  // Block UI thread until a new Runtime instance is created.
  Runtime* WaitForSingleNewRuntime() {
    notification_observer_->Wait();
    const RuntimeList& runtime_list = runtimes();
    for (RuntimeList::const_iterator it = runtime_list.begin();
         it != runtime_list.end(); ++it) {
      RuntimeList::iterator target =
          std::find(original_runtimes_.begin(), original_runtimes_.end(), *it);
      // Not found means a new one.
      if (target == original_runtimes_.end()) {
        original_runtimes_.push_back(*it);
        return *it;
      }
    }
    return NULL;
  }

 private:
  RuntimeList original_runtimes_;
  scoped_ptr<content::WindowedNotificationObserver> notification_observer_;
};

// FIXME(hmin): Since currently the browser process is not shared by multiple
// app launch, this test is disabled to avoid floody launches.
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, DISABLED_SecondLaunch) {
  Relaunch(GetCommandLineForRelaunch());

  Runtime* second_runtime = NULL;
  EXPECT_TRUE(second_runtime == WaitForSingleNewRuntime());
  ASSERT_EQ(2u, runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, CreateAndCloseRuntime) {
  size_t len = runtimes().size();
  ASSERT_EQ(1, len);

  // Create a new Runtime instance.
  GURL url(test_server()->GetURL("test.html"));
  Runtime* new_runtime = Runtime::CreateWithDefaultWindow(
      runtime()->runtime_context(), url, runtime_registry());
  EXPECT_TRUE(url == new_runtime->web_contents()->GetURL());
  EXPECT_EQ(new_runtime, WaitForSingleNewRuntime());
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len + 1, runtimes().size());

  // Close the newly created Runtime instance.
  new_runtime->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len, runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LoadURLAndClose) {
  GURL url(test_server()->GetURL("test.html"));
  size_t len = runtimes().size();
  runtime()->LoadURL(url);
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len, runtimes().size());
  runtime()->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len - 1, runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, CloseNativeWindow) {
  GURL url(test_server()->GetURL("test.html"));
  Runtime* new_runtime = Runtime::CreateWithDefaultWindow(
      runtime()->runtime_context(), url, runtime_registry());
  size_t len = runtimes().size();
  new_runtime->window()->Close();
  content::RunAllPendingInMessageLoop();
  // Closing native window will lead to close Runtime instance.
  EXPECT_EQ(len - 1, runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LaunchWithFullscreenWindow) {
  GURL url(test_server()->GetURL("test.html"));
  Runtime* new_runtime = Runtime::Create(
      runtime()->runtime_context(), runtime_registry());

  NativeAppWindow::CreateParams params;
  params.state = ui::SHOW_STATE_FULLSCREEN;
  new_runtime->AttachWindow(params);
  xwalk_test_utils::NavigateToURL(new_runtime, url);

  EXPECT_TRUE(new_runtime->window()->IsFullscreen());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, HTML5FullscreenAPI) {
  size_t len = runtimes().size();
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("fullscreen.html"));
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_TRUE(false == runtime()->window()->IsFullscreen());

  FullscreenNotificationObserver enter_observer;
  bool ret = content::ExecuteScript(runtime()->web_contents(), "doFullscreenClick();");
  EXPECT_TRUE(ret);
  content::RunAllPendingInMessageLoop();
  enter_observer.Wait();
  // Calling doFullscreenClick defined in fullscreen.html leads to enter into
  // fullscreen window state, so it's expected to be fullscreen.
  EXPECT_TRUE(true == runtime()->window()->IsFullscreen());

  FullscreenNotificationObserver exit_observer;
  ret = content::ExecuteScript(runtime()->web_contents(), "doExitFullscreenClick();");
  EXPECT_TRUE(ret);
  content::RunAllPendingInMessageLoop();
  exit_observer.Wait();
  // Calling doExitFullscreenClick defined in fullscreen.html leads to exit
  // fullscreen window state, so it's expected to be not fullscreen.
  EXPECT_TRUE(false == runtime()->window()->IsFullscreen());
}


#if !defined(OS_MACOSX)
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, GetWindowTitle) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("title.html"));
  base::string16 title = base::ASCIIToUTF16("Dummy Title");
  content::TitleWatcher title_watcher(runtime()->web_contents(), title);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());

  NativeAppWindow* window = runtime()->window();
  base::string16 window_title = window->GetNativeWindow()->title();
  EXPECT_EQ(title, window_title);
}
#endif

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, OpenLinkInNewRuntime) {
  size_t len = runtimes().size();
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("new_target.html"));
  xwalk_test_utils::NavigateToURL(runtime(), url);
  bool ret = content::ExecuteScript(runtime()->web_contents(), "doClick();");
  EXPECT_TRUE(ret);
  content::RunAllPendingInMessageLoop();
  // Calling doClick defined in new_target.html leads to open a href in a new
  // target window, and so it is expected to create a new Runtime instance.
  Runtime* second = WaitForSingleNewRuntime();
  EXPECT_TRUE(NULL != second);
  EXPECT_NE(runtime(), second);
  EXPECT_EQ(len + 1, runtimes().size());
}

#if defined(OS_TIZEN)
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LoadTizenWebUiFwFile) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("tizenwebuifw.html"));
  base::string16 title = base::ASCIIToUTF16("Pass");
  content::TitleWatcher title_watcher(runtime()->web_contents(), title);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());
}
#endif
