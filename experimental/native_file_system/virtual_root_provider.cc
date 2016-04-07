// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/native_file_system/virtual_root_provider.h"

#include <algorithm>
#include <cctype>
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
  std::string uppercase_virtual_root = virtual_root;
  std::transform(virtual_root.begin(), virtual_root.end(),
                 uppercase_virtual_root.begin(), ::toupper);
  return virtual_root_map_[uppercase_virtual_root].AsUTF8Unsafe();
}

VirtualRootProvider::~VirtualRootProvider() {}
