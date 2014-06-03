// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/native_file_system/virtual_root_provider.h"

#include <map>
#include <string>

#include "base/file_util.h"
#include "base/files/file_path.h"

bool VirtualRootProvider::testing_enabled_ = false;

VirtualRootProvider::VirtualRootProvider() {
  if (!virtual_root_map_.empty())
    return;

  if (testing_enabled_) {
    base::GetTempDir(&home_path_);
  } else {
    home_path_ = base::GetHomeDir();
  }

  // TODO(shawngao5): Hard code virtual root mapping for linux here.
  // In future, it should be removed by parsing xdg configure file for
  // user direcory.
  virtual_root_map_["DESKTOP"] = home_path_.Append("Desktop");
  virtual_root_map_["DOWNLOADS"] = home_path_.Append("Downloads");
  virtual_root_map_["DOCUMENTS"] = home_path_.Append("Documents");
  virtual_root_map_["MUSIC"] = home_path_.Append("Music");
  virtual_root_map_["PICTURES"] = home_path_.Append("Pictures");
  virtual_root_map_["VIDEOS"] = home_path_.Append("Videos");
}

void VirtualRootProvider::SetTesting(bool testing_enabled) {
  testing_enabled_ = testing_enabled;
  base::FilePath tmp_path;
  base::GetTempDir(&tmp_path);
  base::FilePath doc_path = tmp_path.Append("Documents");
  if (!DirectoryExists(doc_path)) {
    CreateDirectory(doc_path);
  }
}
