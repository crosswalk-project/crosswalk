// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/display_info_provider.h"

#include <vector>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/layout.h"
#include "ui/gfx/screen.h"
#include "xwalk/sysapps/common/sysapps_manager.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities.h"

#if defined(USE_AURA)
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#endif

using xwalk::jsapi::device_capabilities::DisplayUnit;
using xwalk::jsapi::device_capabilities::SystemDisplay;
using xwalk::sysapps::DisplayInfoProvider;
using xwalk::sysapps::SysAppsManager;

namespace {

class TestObserver : public DisplayInfoProvider::Observer {
 public:
  TestObserver() {}

 private:
  void OnDisplayConnected(const DisplayUnit& display) override {}
  void OnDisplayDisconnected(const DisplayUnit& display) override {}
};

}  // namespace

TEST(XWalkSysAppsDeviceCapabilitiesTest, DisplayInfoProvider) {
  base::MessageLoop message_loop(base::MessageLoop::TYPE_UI);

#if defined(USE_AURA)
  std::vector<ui::ScaleFactor> supported_scale_factors;
  supported_scale_factors.push_back(ui::SCALE_FACTOR_200P);
  ui::SetSupportedScaleFactors(supported_scale_factors);

  gfx::Screen::SetScreenInstance(views::CreateDesktopScreen());
#endif

  DisplayInfoProvider* provider(SysAppsManager::GetDisplayInfoProvider());
  EXPECT_TRUE(provider != NULL);

  std::unique_ptr<SystemDisplay> info(provider->display_info());
  EXPECT_TRUE(info != NULL);

  std::vector<DisplayUnit> displays = std::move(info->displays);

  size_t display_count = displays.size();
  EXPECT_GE(display_count, 0u);

  for (size_t i = 0; i < display_count; ++i) {
    EXPECT_FALSE(displays[i].id.empty());
    EXPECT_GE(displays[i].width, 0);
    EXPECT_GE(displays[i].height, 0);
    EXPECT_GE(displays[i].avail_width, 0);
    EXPECT_GE(displays[i].avail_height, 0);
  }

  TestObserver test_observer;
  provider->AddObserver(&test_observer);
  provider->RemoveObserver(&test_observer);

  message_loop.PostTask(FROM_HERE,
                        Bind(&base::MessageLoop::QuitWhenIdle,
                             Unretained(base::MessageLoop::current())));

  message_loop.Run();
}
