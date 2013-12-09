// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_H_
#define XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_H_

#include <string>
#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/image_view.h"

namespace xwalk {

class TizenSystemIndicatorWatcher;

// This view paints the Tizen Mobile system indicator provided by the system.
// We get to it by using the Elementary "Plug" system from EFL, reading the
// image from a shared memory area.
class TizenSystemIndicator : public views::ImageView {
 public:
  TizenSystemIndicator();
  virtual ~TizenSystemIndicator();

  bool IsConnected() const;

  enum Orientation { LANDSCAPE, PORTRAIT };
  void SetOrientation(Orientation orientation);
  Orientation GetOrientation() const;

  // views::View implementation.
  gfx::Size GetPreferredSize() OVERRIDE;

 private:
  virtual bool OnMousePressed(const ui::MouseEvent& event) OVERRIDE;
  virtual void OnMouseReleased(const ui::MouseEvent& event) OVERRIDE;
  virtual void OnMouseMoved(const ui::MouseEvent& event) OVERRIDE;
  virtual void OnTouchEvent(ui::TouchEvent* event) OVERRIDE;

  Orientation orientation_;
  scoped_ptr<TizenSystemIndicatorWatcher> watcher_;
  friend class TizenSystemIndicatorWatcher;
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_MOBILE_UI_TIZEN_SYSTEM_INDICATOR_H_
