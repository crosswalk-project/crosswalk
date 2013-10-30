// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define IPC_MESSAGE_IMPL
#include "xwalk/extensions/common/xwalk_extension_messages.h"

// Generate constructors.
#include "ipc/struct_constructor_macros.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h" // NOLINT(*)

// Generate destructors.
#include "ipc/struct_destructor_macros.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h" // NOLINT(*)

// Generate param traits write methods.
#include "ipc/param_traits_write_macros.h"
namespace IPC {
#include "xwalk/extensions/common/xwalk_extension_messages.h" // NOLINT(*)
}  // namespace IPC

// Generate param traits write methods.
#include "ipc/param_traits_read_macros.h"
namespace IPC {
#include "xwalk/extensions/common/xwalk_extension_messages.h" // NOLINT(*)
}  // namespace IPC

// Generate param traits log methods.
#include "ipc/param_traits_log_macros.h"
namespace IPC {
#include "xwalk/extensions/common/xwalk_extension_messages.h" // NOLINT(*)
}  // namespace IPC
