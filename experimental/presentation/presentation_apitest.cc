// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/experimental/presentation/presentation_display_manager.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "ui/gfx/display.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionService;
using xwalk::experimental::PresentationDisplayManager;

namespace xwalk {
namespace experimental {

class PresentationExtensionTest : public XWalkExtensionsTestBase {
 public:
  // Simulate a new secondary display is arrived from system.
  void AttachDisplay(const gfx::Display& d) {
    PresentationDisplayManager::GetForTesting()->AddSecondaryDisplay(d);
  }

  // Simulate a secondray display is removed from system.
  void DetachDisplay(const gfx::Display& d) {
    PresentationDisplayManager::GetForTesting()->RemoveSecondaryDisplay(d);
  }
};

IN_PROC_BROWSER_TEST_F(PresentationExtensionTest, DisplayAvailable) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(
      base::FilePath().AppendASCII("presentation_api"),
      base::FilePath().AppendASCII("display_available.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(PresentationExtensionTest, DisplayAvailableChange) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(
      base::FilePath().AppendASCII("presentation_api"),
      base::FilePath().AppendASCII("display_available_change.html"));

  gfx::Display display1(100);
  gfx::Display display2(200);

  {
    // Case 1: When the first secondary display is attached, the event
    // displayavailablechange will be fired.
    string16 expected_title = ASCIIToUTF16("True:1");
    content::TitleWatcher title_watcher(
        runtime()->web_contents(), expected_title);
    xwalk_test_utils::NavigateToURL(runtime(), url);
    AttachDisplay(display1);
    EXPECT_EQ(expected_title, title_watcher.WaitAndGetTitle());
  }

  {
    // Case 2: When the second secondary display is attached, the event
    // displayavailablechange will not be fired.
    string16 expected_title = ASCIIToUTF16("True:1");
    AttachDisplay(display2);
    content::RunAllPendingInMessageLoop();
    const string16& real_title = runtime()->web_contents()->GetTitle();
    EXPECT_EQ(expected_title, real_title);
  }

  {
    // Case 3: When one of secondary displays is detached, the event
    // displayavailablechange also won't be fired.
    string16 expected_title = ASCIIToUTF16("True:1");
    DetachDisplay(display1);
    content::RunAllPendingInMessageLoop();
    const string16& real_title = runtime()->web_contents()->GetTitle();
    EXPECT_EQ(expected_title, real_title);
  }

  {
    // Case 4: when the last secondary display is detached, the event
    // displayavailablechange  event will be fired.
    string16 expected_title = ASCIIToUTF16("False:1");
    content::TitleWatcher title_watcher(
        runtime()->web_contents(), expected_title);
    DetachDisplay(display2);
    EXPECT_EQ(expected_title, title_watcher.WaitAndGetTitle());
  }
}

}  // namespace experimental
}  // namespace xwalk
