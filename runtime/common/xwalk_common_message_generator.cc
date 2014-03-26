// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Get basic type definitions.
#define IPC_MESSAGE_IMPL
#include "xwalk/runtime/common/xwalk_common_message_generator.h"

// Generate constructors.
#include "ipc/struct_constructor_macros.h"
#include "xwalk/runtime/common/xwalk_common_message_generator.h"  // NOLINT

// Generate destructors.
#include "ipc/struct_destructor_macros.h"
#include "xwalk/runtime/common/xwalk_common_message_generator.h"  // NOLINT

// Generate param traits write methods.
#include "ipc/param_traits_write_macros.h"
namespace IPC {
#include "xwalk/runtime/common/xwalk_common_message_generator.h"  // NOLINT
}  // namespace IPC

// Generate param traits read methods.
#include "ipc/param_traits_read_macros.h"
namespace IPC {
#include "xwalk/runtime/common/xwalk_common_message_generator.h"  // NOLINT
}  // namespace IPC

// Generate param traits log methods.
#include "ipc/param_traits_log_macros.h"
namespace IPC {
#include "xwalk/runtime/common/xwalk_common_message_generator.h"  // NOLINT
}  // namespace IPC
