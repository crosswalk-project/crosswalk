// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_resource_delegate.h"

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "ui/gfx/image/image.h"

namespace xwalk {

XWalkResourceDelegate::XWalkResourceDelegate() {}

XWalkResourceDelegate::~XWalkResourceDelegate() {}

base::FilePath XWalkResourceDelegate::GetPathForResourcePack(
    const base::FilePath& pack_path,
    ui::ScaleFactor scale_factor) {
  return pack_path;
}

base::FilePath XWalkResourceDelegate::GetPathForLocalePack(
    const base::FilePath& pack_path,
    const std::string& locale) {
  base::FilePath product_dir;
  if (!PathService::Get(base::DIR_MODULE, &product_dir)) {
    NOTREACHED();
  }
  return product_dir.
      Append(FILE_PATH_LITERAL("locales")).
      Append(FILE_PATH_LITERAL("xwalk")).
      AppendASCII(locale + ".pak");
}

gfx::Image XWalkResourceDelegate::GetImageNamed(int resource_id) {
  return gfx::Image();
}

gfx::Image XWalkResourceDelegate::GetNativeImageNamed(int resource_id) {
  return gfx::Image();
}

base::RefCountedStaticMemory* XWalkResourceDelegate::LoadDataResourceBytes(
    int resource_id,
    ui::ScaleFactor scale_factor) {
  return nullptr;
}

bool XWalkResourceDelegate::GetRawDataResource(int resource_id,
                                               ui::ScaleFactor scale_factor,
                                               base::StringPiece* value) {
  return false;
}

bool XWalkResourceDelegate::GetLocalizedString(int message_id,
                                               base::string16* value) {
  return false;
}

}  // namespace xwalk
