// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/display_observer.h"

namespace gfx {

// FIXME(tmpsantos): We can probably remove this hack after fixing
// https://code.google.com/p/chromium/issues/detail?id=326140
DisplayObserver::~DisplayObserver() {}

}  // namespace gfx
