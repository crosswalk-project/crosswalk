// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_VIRTUAL_ROOT_PROVIDER_H_
#define XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_VIRTUAL_ROOT_PROVIDER_H_

#include <map>
#include <string>

#include "base/files/file_path.h"

namespace base {

template <typename Type>
struct DefaultLazyInstanceTraits;

}  // namespace base

class VirtualRootProvider {
 public:
  static VirtualRootProvider* GetInstance();
  std::string GetRealPath(const std::string& virtual_root);
#if defined(OS_LINUX)
  static void SetTesting(bool test);
#endif

 private:
  friend struct base::DefaultLazyInstanceTraits<VirtualRootProvider>;
  VirtualRootProvider();
  ~VirtualRootProvider();

  base::FilePath home_path_;
  std::map<std::string, base::FilePath> virtual_root_map_;
#if defined(OS_LINUX)
  static bool testing_enabled_;
#endif

  DISALLOW_COPY_AND_ASSIGN(VirtualRootProvider);
};

#endif  // XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_VIRTUAL_ROOT_PROVIDER_H_
