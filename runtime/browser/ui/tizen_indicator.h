// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_TIZEN_INDICATOR_H_
#define XWALK_RUNTIME_BROWSER_UI_TIZEN_INDICATOR_H_

#include <string>
#include "ui/gfx/image/image_skia.h"
#include "ui/views/view.h"

namespace xwalk {

class TizenPlug;

// This view paints the Tizen Mobile indicator provided by the system. We get
// to it by using the elementary "plug" system from EFL, reading the image from
// a shared memory area.
class TizenIndicator : public views::View {
 public:
  TizenIndicator();
  virtual ~TizenIndicator();

  bool IsConnected() const;

  // views::View implementation.
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  gfx::Size GetPreferredSize() OVERRIDE;

 private:
  // Will be called by plug when the image is updated.
  void SetImage(const gfx::ImageSkia& img);

  gfx::ImageSkia image_;
  scoped_ptr<TizenPlug> plug_;
  friend class TizenPlug;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_TIZEN_INDICATOR_H_
