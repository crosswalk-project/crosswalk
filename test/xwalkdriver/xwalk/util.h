// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_UTIL_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_UTIL_H_

#include <string>

namespace base {
class Value;
}

std::string SerializeValue(const base::Value* value);

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_UTIL_H_
