// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_XWALK_KEY_SYSTEMS_H_
#define XWALK_RUNTIME_RENDERER_XWALK_KEY_SYSTEMS_H_

#include <vector>

#include "media/base/key_system_info.h"

namespace xwalk {

void AddXwalkKeySystems(std::vector<media::KeySystemInfo>* key_systems_info);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_XWALK_KEY_SYSTEMS_H_
