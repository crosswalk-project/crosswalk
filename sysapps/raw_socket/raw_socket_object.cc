// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/raw_socket/raw_socket_object.h"

#include <memory>

#include "base/values.h"

namespace xwalk {
namespace sysapps {

RawSocketObject::RawSocketObject() {}

RawSocketObject::~RawSocketObject() {}

void RawSocketObject::setReadyState(ReadyState state) {
  std::unique_ptr<base::ListValue> eventData(new base::ListValue);
  eventData->AppendString(ToString(state));

  DispatchEvent("readystate", std::move(eventData));
}

}  // namespace sysapps
}  // namespace xwalk
