// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/display_info_provider.h"

#include <vector>

#include "base/strings/string_number_conversions.h"
#include "base/sys_info.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"

using xwalk::jsapi::device_capabilities::DisplayUnit;

namespace {

// We are calculating the DPI the same way Chromium for
// ChromiumOS. See display_info_provider_chromeos.cc.
const float kDpi96 = 96;

std::unique_ptr<DisplayUnit> makeDisplayUnit(const gfx::Display& display) {
  gfx::Screen* screen = gfx::Screen::GetScreen();
  const int64_t primary_display_id = screen->GetPrimaryDisplay().id();

  std::unique_ptr<DisplayUnit> display_unit(new DisplayUnit);

  display_unit->id = base::Int64ToString(display.id());
  // FIXME(YuZhiqiangX): find which field reflects 'name'.
  display_unit->name = "unknown";

  display_unit->primary = (display.id() == primary_display_id);
  display_unit->external = !display.IsInternal();

  const float dpi = display.device_scale_factor() * kDpi96;
  display_unit->device_xdpi = static_cast<unsigned int>(dpi);
  display_unit->device_ydpi = static_cast<unsigned int>(dpi);

  display_unit->width = display.bounds().width();
  display_unit->height = display.bounds().height();
  display_unit->avail_width = display.work_area_size().width();
  display_unit->avail_height = display.work_area_size().height();

  // colorDepth and pixelDepth are constantly set as 24 refer to
  // http://www.w3.org/TR/cssom-view/#dom-screen-colordepth
  display_unit->color_depth = 24;
  display_unit->pixel_depth = 24;

  return display_unit;
}

}  // namespace

namespace xwalk {
namespace sysapps {

DisplayInfoProvider::DisplayInfoProvider() {}

DisplayInfoProvider::~DisplayInfoProvider() {}

// static
std::unique_ptr<SystemDisplay> DisplayInfoProvider::display_info() {
  std::unique_ptr<SystemDisplay> info(new SystemDisplay);

  gfx::Screen* screen = gfx::Screen::GetScreen();
  std::vector<gfx::Display> displays = screen->GetAllDisplays();

  for (std::vector<gfx::Display>::const_iterator it = displays.begin();
      it != displays.end(); ++it) {
    auto display_unit = makeDisplayUnit(*it);
    info->displays.push_back(std::move(*display_unit));
  }

  return info;
}

void DisplayInfoProvider::AddObserver(Observer* observer) {
  bool should_start_monitoring = false;
  if (!observer_list_.might_have_observers())
    should_start_monitoring = true;

  observer_list_.AddObserver(observer);

  if (should_start_monitoring)
    StartDisplayMonitoring();
}

void DisplayInfoProvider::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);

  if (!observer_list_.might_have_observers())
    StopDisplayMonitoring();
}

bool DisplayInfoProvider::HasObserver(Observer* observer) const {
  return observer_list_.HasObserver(observer);
}

void DisplayInfoProvider::StartDisplayMonitoring() {
  gfx::Screen* screen = gfx::Screen::GetScreen();
  screen->AddObserver(this);
}

void DisplayInfoProvider::StopDisplayMonitoring() {
  gfx::Screen* screen = gfx::Screen::GetScreen();
  screen->RemoveObserver(this);
}

void DisplayInfoProvider::OnDisplayAdded(const gfx::Display& display) {
  FOR_EACH_OBSERVER(Observer,
                    observer_list_,
                    OnDisplayConnected(*makeDisplayUnit(display)));
}

void DisplayInfoProvider::OnDisplayRemoved(const gfx::Display& display) {
  FOR_EACH_OBSERVER(Observer,
                    observer_list_,
                    OnDisplayDisconnected(*makeDisplayUnit(display)));
}

}  // namespace sysapps
}  // namespace xwalk
