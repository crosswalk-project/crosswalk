// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/native_file_system/virtual_root_provider.h"

#include <shlobj.h>

#include "base/files/file_path.h"
#include "base/files/file_util.h"

bool VirtualRootProvider::testing_enabled_ = false;

VirtualRootProvider::VirtualRootProvider() {
  if (testing_enabled_) {
    base::FilePath tmp_path;
    base::GetTempDir(&tmp_path);
    base::FilePath doc_path = tmp_path.Append(FILE_PATH_LITERAL("Documents"));
    if (!DirectoryExists(doc_path))
      CreateDirectory(doc_path);
    virtual_root_map_["DOCUMENTS"] = doc_path;
    return;
  }

  LPWSTR wszPath = nullptr;
  HRESULT hr = SHGetKnownFolderPath(FOLDERID_Desktop,
                                    KF_FLAG_DONT_UNEXPAND, nullptr, &wszPath);
  if (SUCCEEDED(hr)) {
    virtual_root_map_["DESKTOP"] = base::FilePath(wszPath);
    CoTaskMemFree(wszPath);
  }

  hr = SHGetKnownFolderPath(FOLDERID_Documents,
                            KF_FLAG_DONT_UNEXPAND, nullptr, &wszPath);
  if (SUCCEEDED(hr)) {
    virtual_root_map_["DOCUMENTS"] = base::FilePath(wszPath);
    CoTaskMemFree(wszPath);
  }

  hr = SHGetKnownFolderPath(FOLDERID_Music,
                            KF_FLAG_DONT_UNEXPAND, nullptr, &wszPath);
  if (SUCCEEDED(hr)) {
    virtual_root_map_["MUSIC"] = base::FilePath(wszPath);
    CoTaskMemFree(wszPath);
  }

  hr = SHGetKnownFolderPath(FOLDERID_Pictures,
                            KF_FLAG_DONT_UNEXPAND, nullptr, &wszPath);
  if (SUCCEEDED(hr)) {
    virtual_root_map_["PICTURES"] = base::FilePath(wszPath);
    CoTaskMemFree(wszPath);
  }

  hr = SHGetKnownFolderPath(FOLDERID_Videos,
                            KF_FLAG_DONT_UNEXPAND, nullptr, &wszPath);
  if (SUCCEEDED(hr)) {
    virtual_root_map_["VIDEOS"] = base::FilePath(wszPath);
    CoTaskMemFree(wszPath);
  }

  hr = SHGetKnownFolderPath(FOLDERID_Downloads,
                            KF_FLAG_DONT_UNEXPAND, nullptr, &wszPath);
  if (SUCCEEDED(hr)) {
    virtual_root_map_["DOWNLOADS"] = base::FilePath(wszPath);
    CoTaskMemFree(wszPath);
  }
}

void VirtualRootProvider::SetTesting(bool testing_enabled) {
  testing_enabled_ = testing_enabled;
}
