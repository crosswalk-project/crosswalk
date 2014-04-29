// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_STORAGE_COMPONENT_H_
#define XWALK_RUNTIME_BROWSER_STORAGE_COMPONENT_H_

#include "xwalk/runtime/browser/xwalk_component.h"
#include "xwalk/experimental/native_file_system/native_file_system_extension.h"

namespace xwalk {

class StorageComponent : public XWalkComponent {
 public:
  StorageComponent();
  virtual ~StorageComponent();

 private:
  // XWalkComponent implementation.
  virtual void CreateExtensionThreadExtensions(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) OVERRIDE;

  experimental::NativeFileSystemExtension* native_file_system_extension_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_STORAGE_COMPONENT_H_
