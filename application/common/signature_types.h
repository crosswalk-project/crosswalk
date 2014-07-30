// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_SIGNATURE_TYPES_H_
#define XWALK_APPLICATION_COMMON_SIGNATURE_TYPES_H_

#include <map>
#include <string>

namespace xwalk {
namespace application {

struct ReferenceData {
  std::string transform_algorithm;
  std::string digest_method;
  std::string digest_value;
};

typedef std::map<std::string, ReferenceData> ReferenceHashMap;
}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_SIGNATURE_TYPES_H_
