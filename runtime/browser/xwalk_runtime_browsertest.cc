// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_ui_delegate.h"
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
using content::NotificationService;
using content::WebContents;
using content::WindowedNotificationObserver;
using testing::_;

class XWalkRuntimeTest : public InProcessBrowserTest {
};

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, CreateAndCloseRuntime) {
  size_t len = runtimes().size();
  // Create a new Runtime instance.
  GURL url(test_server()->GetURL("test.html"));
  Runtime* runtime = CreateRuntime(url);
  EXPECT_TRUE(url == runtime->web_contents()->GetURL());
  EXPECT_EQ(len + 1, runtimes().size());

  // Close the newly created Runtime instance.
  runtime->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len, runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LoadURLAndClose) {
  GURL url(test_server()->GetURL("test.html"));
  Runtime* runtime = CreateRuntime(url);
  size_t len = runtimes().size();
  runtime->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len - 1, runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, CloseNativeWindow) {
  GURL url(test_server()->GetURL("test.html"));
  Runtime* new_runtime = CreateRuntime(url);
  size_t len = runtimes().size();
  new_runtime->window()->Close();
  content::RunAllPendingInMessageLoop();
  // Closing native window will lead to close Runtime instance.
  EXPECT_EQ(len - 1, runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LaunchWithFullscreenWindow) {
  GURL url(test_server()->GetURL("test.html"));
  NativeAppWindow::CreateParams params;
  params.state = ui::SHOW_STATE_FULLSCREEN;
  Runtime* new_runtime = CreateRuntime(url, params);

  EXPECT_TRUE(new_runtime->window()->IsFullscreen());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, HTML5FullscreenAPI) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("fullscreen.html"));
  Runtime* runtime = CreateRuntime(url);
  EXPECT_TRUE(false == runtime->window()->IsFullscreen());

  WindowedNotificationObserver enter_observer(
     xwalk::NOTIFICATION_FULLSCREEN_CHANGED,
     NotificationService::AllSources());

  bool ret = content::ExecuteScript(
      runtime->web_contents(), "doFullscreenClick();");
  EXPECT_TRUE(ret);
  content::RunAllPendingInMessageLoop();
  enter_observer.Wait();
  // Calling doFullscreenClick defined in fullscreen.html leads to enter into
  // fullscreen window state, so it's expected to be fullscreen.
  EXPECT_TRUE(true == runtime->window()->IsFullscreen());

  WindowedNotificationObserver exit_observer(
     xwalk::NOTIFICATION_FULLSCREEN_CHANGED,
     NotificationService::AllSources());

  ret = content::ExecuteScript(
      runtime->web_contents(), "doExitFullscreenClick();");
  EXPECT_TRUE(ret);
  content::RunAllPendingInMessageLoop();
  exit_observer.Wait();
  // Calling doExitFullscreenClick defined in fullscreen.html leads to exit
  // fullscreen window state, so it's expected to be not fullscreen.
  EXPECT_TRUE(false == runtime->window()->IsFullscreen());
}


#if !defined(OS_MACOSX)
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, GetWindowTitle) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("title.html"));
  base::string16 title = base::ASCIIToUTF16("Dummy Title");
  Runtime* runtime = CreateRuntime();
  content::TitleWatcher title_watcher(runtime->web_contents(), title);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());

  NativeAppWindow* window = runtime->window();
  base::string16 window_title = window->GetNativeWindow()->title();
  EXPECT_EQ(title, window_title);
}
#endif

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, OpenLinkInNewRuntime) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("new_target.html"));
  Runtime* runtime = CreateRuntime(url);
  size_t len = runtimes().size();
  bool ret = content::ExecuteScript(runtime->web_contents(), "doClick();");
  EXPECT_TRUE(ret);
  content::RunAllPendingInMessageLoop();
  // Calling doClick defined in new_target.html leads to open a href in a new
  // target window, and so it is expected to create a new Runtime instance.
  EXPECT_EQ(len + 1, runtimes().size());
  Runtime* second = runtimes().back();
  EXPECT_TRUE(NULL != second);
  EXPECT_NE(runtime, second);
}

#if defined(OS_TIZEN)
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LoadTizenWebUiFwFile) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("tizenwebuifw.html"));
  base::string16 title = base::ASCIIToUTF16("Pass");
  Runtime* runtime = CreateRuntime();
  content::TitleWatcher title_watcher(runtime->web_contents(), title);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());
}
#endif
