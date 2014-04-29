// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/native_file_system/virtual_root_provider.h"

#include <map>
#include <string>

#include "base/lazy_instance.h"
#include "base/macros.h"

namespace {

base::LazyInstance<VirtualRootProvider>::Leaky g_lazy_instance;

}  // namespace

VirtualRootProvider* VirtualRootProvider::GetInstance() {
  return g_lazy_instance.Pointer();
}

std::string VirtualRootProvider::GetRealPath(const std::string& virtual_root) {
  return virtual_root_map_[virtual_root].AsUTF8Unsafe();
}

VirtualRootProvider::~VirtualRootProvider() {}
