// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"

#if defined(COMPONENT_BUILD) && !defined(COMPILE_CONTENT_STATICALLY)

#if defined(COMPILER_MSVC)
#pragma warning(disable:4273)
#endif

class GURL;

namespace {
class DummyContentClient {
 public:
  void SetActiveURL(const GURL&) {}
};
static DummyContentClient _dummy_client;
}

#define GetContentClient() (&_dummy_client)

// Avoid redefinition error
#include "content/common/plugin_messages.h"
#include "ipc/ipc_message_null_macros.h"

#include "content/common/message_router.cc"
#include "content/common/np_channel_base.cc"
#include "content/common/npobject_proxy.cc"
#include "content/common/npobject_stub.cc"
#include "content/common/npobject_util.cc"
#include "content/common/content_param_traits.cc"  // must be the last

#endif  // defined(COMPONENT_BUILD)
