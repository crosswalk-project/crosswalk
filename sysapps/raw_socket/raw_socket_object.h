// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_OBJECT_H_
#define XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_OBJECT_H_

#include "xwalk/sysapps/raw_socket/raw_socket.h"
#include "xwalk/sysapps/common/event_target.h"

using xwalk::jsapi::raw_socket::ReadyState; // NOLINT

namespace xwalk {
namespace sysapps {

// Base class for the objects of the RawSocket API.
class RawSocketObject : public EventTarget {
 public:
  virtual ~RawSocketObject();

 protected:
  RawSocketObject();

  void setReadyState(ReadyState state);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_OBJECT_H_
