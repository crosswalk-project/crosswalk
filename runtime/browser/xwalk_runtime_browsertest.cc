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
#include "xwalk/runtime/browser/runtime_registry.h"
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
using xwalk::RuntimeRegistry;
using xwalk::RuntimeList;
using content::WebContents;
using testing::_;

// A mock observer to listen runtime registry changes.
class MockRuntimeRegistryObserver : public xwalk::RuntimeRegistryObserver {
 public:
  MockRuntimeRegistryObserver() {}
  virtual ~MockRuntimeRegistryObserver() {}

  MOCK_METHOD1(OnRuntimeAdded, void(Runtime* runtime));
  MOCK_METHOD1(OnRuntimeRemoved, void(Runtime* runtime));
  MOCK_METHOD1(OnRuntimeAppIconChanged, void(Runtime* runtime));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockRuntimeRegistryObserver);
};

// An observer used to verify app icon change.
class FaviconChangedObserver : public xwalk::RuntimeRegistryObserver {
 public:
  explicit FaviconChangedObserver(const base::FilePath& icon_file)
      : icon_file_(icon_file) {
  }

  virtual ~FaviconChangedObserver() {}

  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE {}

  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE {}

  virtual void OnRuntimeAppIconChanged(Runtime* runtime) OVERRIDE {
    const base::FilePath::StringType kPNGFormat(FILE_PATH_LITERAL(".png"));
    const base::FilePath::StringType kICOFormat(FILE_PATH_LITERAL(".ico"));

    gfx::Image image;
    image = xwalk_utils::LoadImageFromFilePath(icon_file_);

    EXPECT_FALSE(image.IsEmpty());
    gfx::Image icon = runtime->app_icon();
    EXPECT_FALSE(icon.IsEmpty());
    EXPECT_EQ(image.Size(), icon.Size());

    // Quit the message loop.
    base::MessageLoop::current()->PostTask(
        FROM_HERE, base::MessageLoop::QuitClosure());
  }

 private:
  const base::FilePath& icon_file_;
};

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
    const RuntimeList& runtimes = RuntimeRegistry::Get()->runtimes();
    for (RuntimeList::const_iterator it = runtimes.begin();
         it != runtimes.end(); ++it)
      original_runtimes_.push_back(*it);
  }

  // Block UI thread until a new Runtime instance is created.
  Runtime* WaitForSingleNewRuntime() {
    notification_observer_->Wait();
    const RuntimeList& runtimes = RuntimeRegistry::Get()->runtimes();
    for (RuntimeList::const_iterator it = runtimes.begin();
         it != runtimes.end(); ++it) {
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
  MockRuntimeRegistryObserver observer;
  RuntimeRegistry::Get()->AddObserver(&observer);
  Relaunch(GetCommandLineForRelaunch());

  Runtime* second_runtime = NULL;
  EXPECT_TRUE(second_runtime == WaitForSingleNewRuntime());
  EXPECT_CALL(observer, OnRuntimeAdded(second_runtime)).Times(1);
  ASSERT_EQ(2u, RuntimeRegistry::Get()->runtimes().size());

  RuntimeRegistry::Get()->RemoveObserver(&observer);
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, CreateAndCloseRuntime) {
  MockRuntimeRegistryObserver observer;
  RuntimeRegistry::Get()->AddObserver(&observer);
  // At least one Runtime instance is created at startup.
  size_t len = RuntimeRegistry::Get()->runtimes().size();
  ASSERT_EQ(1, len);

  // Create a new Runtime instance.
  GURL url(test_server()->GetURL("test.html"));
  EXPECT_CALL(observer, OnRuntimeAdded(_)).Times(1);
  Runtime* new_runtime = Runtime::CreateWithDefaultWindow(
      runtime()->runtime_context(), url);
  EXPECT_TRUE(url == new_runtime->web_contents()->GetURL());
  EXPECT_EQ(new_runtime, WaitForSingleNewRuntime());
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len + 1, RuntimeRegistry::Get()->runtimes().size());

  // Close the newly created Runtime instance.
  EXPECT_CALL(observer, OnRuntimeRemoved(new_runtime)).Times(1);
  new_runtime->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len, RuntimeRegistry::Get()->runtimes().size());

  RuntimeRegistry::Get()->RemoveObserver(&observer);
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LoadURLAndClose) {
  GURL url(test_server()->GetURL("test.html"));
  size_t len = RuntimeRegistry::Get()->runtimes().size();
  runtime()->LoadURL(url);
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len, RuntimeRegistry::Get()->runtimes().size());
  runtime()->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len - 1, RuntimeRegistry::Get()->runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, CloseNativeWindow) {
  GURL url(test_server()->GetURL("test.html"));
  Runtime* new_runtime = Runtime::CreateWithDefaultWindow(
      runtime()->runtime_context(), url);
  size_t len = RuntimeRegistry::Get()->runtimes().size();
  new_runtime->window()->Close();
  content::RunAllPendingInMessageLoop();
  // Closing native window will lead to close Runtime instance.
  EXPECT_EQ(len - 1, RuntimeRegistry::Get()->runtimes().size());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LaunchWithFullscreenWindow) {
  MockRuntimeRegistryObserver observer;
  RuntimeRegistry::Get()->AddObserver(&observer);
  // At least one Runtime instance is created at startup.
  size_t len = RuntimeRegistry::Get()->runtimes().size();
  ASSERT_EQ(1, len);
  // Original Runtime should has non fullscreen window.
  EXPECT_TRUE(false == runtime()->window()->IsFullscreen());

  // Add "--fullscreen" launch argument.
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  cmd_line->AppendSwitch("fullscreen");

  // Create a new Runtime instance.
  FullscreenNotificationObserver fullscreen_observer;
  GURL url(test_server()->GetURL("test.html"));
  EXPECT_CALL(observer, OnRuntimeAdded(_)).Times(1);
  Runtime* new_runtime = Runtime::CreateWithDefaultWindow(
      runtime()->runtime_context(), url);
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len + 1, RuntimeRegistry::Get()->runtimes().size());
  fullscreen_observer.Wait();
  EXPECT_TRUE(true == new_runtime->window()->IsFullscreen());

  // Close the newly created Runtime instance.
  EXPECT_CALL(observer, OnRuntimeRemoved(new_runtime)).Times(1);
  new_runtime->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len, RuntimeRegistry::Get()->runtimes().size());

  RuntimeRegistry::Get()->RemoveObserver(&observer);
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, HTML5FullscreenAPI) {
  size_t len = RuntimeRegistry::Get()->runtimes().size();
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("fullscreen.html"));
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_TRUE(false == runtime()->window()->IsFullscreen());

  FullscreenNotificationObserver enter_observer;
  runtime()->web_contents()->GetRenderViewHost()->ExecuteJavascriptInWebFrame(
      string16(),
      ASCIIToUTF16("doFullscreenClick();"));
  content::RunAllPendingInMessageLoop();
  enter_observer.Wait();
  // Calling doFullscreenClick defined in fullscreen.html leads to enter into
  // fullscreen window state, so it's expected to be fullscreen.
  EXPECT_TRUE(true == runtime()->window()->IsFullscreen());

  FullscreenNotificationObserver exit_observer;
  runtime()->web_contents()->GetRenderViewHost()->ExecuteJavascriptInWebFrame(
      string16(),
      ASCIIToUTF16("doExitFullscreenClick();"));
  content::RunAllPendingInMessageLoop();
  exit_observer.Wait();
  // Calling doExitFullscreenClick defined in fullscreen.html leads to exit
  // fullscreen window state, so it's expected to be not fullscreen.
  EXPECT_TRUE(false == runtime()->window()->IsFullscreen());
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, GetWindowTitle) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("title.html"));
  string16 title = ASCIIToUTF16("Dummy Title");
  content::TitleWatcher title_watcher(runtime()->web_contents(), title);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());

  NativeAppWindow* window = runtime()->window();
  string16 window_title = window->GetNativeWindow()->title();
  EXPECT_EQ(title, window_title);
}

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, OpenLinkInNewRuntime) {
  size_t len = RuntimeRegistry::Get()->runtimes().size();
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("new_target.html"));
  xwalk_test_utils::NavigateToURL(runtime(), url);
  runtime()->web_contents()->GetRenderViewHost()->ExecuteJavascriptInWebFrame(
      string16(),
      ASCIIToUTF16("doClick();"));
  content::RunAllPendingInMessageLoop();
  // Calling doClick defined in new_target.html leads to open a href in a new
  // target window, and so it is expected to create a new Runtime instance.
  Runtime* second = WaitForSingleNewRuntime();
  EXPECT_TRUE(NULL != second);
  EXPECT_NE(runtime(), second);
  EXPECT_EQ(len + 1, RuntimeRegistry::Get()->runtimes().size());
}

#if !defined(USE_AURA)
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, FaviconTest_ICO) {
#else
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, DISABLED_FaviconTest_ICO) {
#endif
  base::FilePath icon_file(xwalk_test_utils::GetTestFilePath(
      base::FilePath(FILE_PATH_LITERAL("favicon")),
      base::FilePath(FILE_PATH_LITERAL("16x16.ico"))));

  FaviconChangedObserver observer(icon_file);
  RuntimeRegistry::Get()->AddObserver(&observer);

  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(FILE_PATH_LITERAL("favicon")),
      base::FilePath().AppendASCII("favicon_ico.html"));

  xwalk_test_utils::NavigateToURL(runtime(), url);
  content::RunMessageLoop();
  RuntimeRegistry::Get()->RemoveObserver(&observer);
}

#if !defined(USE_AURA)
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, FaviconTest_PNG) {
#else
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, DISABLED_FaviconTest_PNG) {
#endif
  base::FilePath icon_file(xwalk_test_utils::GetTestFilePath(
      base::FilePath(FILE_PATH_LITERAL("favicon")),
      base::FilePath(FILE_PATH_LITERAL("48x48.png"))));

  FaviconChangedObserver observer(icon_file);
  RuntimeRegistry::Get()->AddObserver(&observer);

  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(FILE_PATH_LITERAL("favicon")),
      base::FilePath().AppendASCII("favicon_png.html"));

  xwalk_test_utils::NavigateToURL(runtime(), url);
  content::RunMessageLoop();
  RuntimeRegistry::Get()->RemoveObserver(&observer);
}

#if defined(OS_TIZEN_MOBILE)
IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LoadTizenWebUiFwFile) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("tizenwebuifw.html"));
  string16 title = ASCIIToUTF16("Pass");
  content::TitleWatcher title_watcher(runtime()->web_contents(), title);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());
}
#endif
