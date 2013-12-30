// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_WIDGET_H_
#define XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_WIDGET_H_

#include "ui/gfx/display.h"
#include "ui/views/widget/widget.h"
#include "xwalk/tizen/mobile/ui/tizen_system_indicator.h"

namespace xwalk {

// In Tizen the system indicator is shown by each application. This widget draws
// the indicator for Crosswalk by waiting for new images from the system (using
// TizenSystemIndicatorWatcher, that uses elm_plug protocol) and setting them
// into the TizenSystemIndicator view, which is the only content of the widget.
class TizenSystemIndicatorWidget : public views::Widget,
    public TizenSystemIndicatorWatcher::WatcherClient {
 public:
  TizenSystemIndicatorWidget();
  virtual ~TizenSystemIndicatorWidget();

  void Initialize(aura::Window* parent);

  // TizenSystemIndicatorWatcher::WatcherClient implementation.
  virtual void OnImageUpdated(const gfx::ImageSkia& img_skia) OVERRIDE;

  // Apply new display configuration.
  void SetDisplay(const gfx::Display& display);

 private:
  scoped_ptr<TizenSystemIndicatorWatcher> watcher_;
  scoped_ptr<TizenSystemIndicator> indicator_;
  friend class TizenSystemIndicator;
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_WIDGET_H_
