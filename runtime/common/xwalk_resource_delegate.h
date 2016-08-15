// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_RESOURCE_DELEGATE_H_
#define XWALK_RUNTIME_COMMON_XWALK_RESOURCE_DELEGATE_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "base/macros.h"
#include "base/memory/ref_counted_memory.h"
#include "ui/base/resource/resource_bundle.h"

namespace base {
class FilePath;
}

namespace gfx {
class Image;
}

namespace xwalk {

// In Chrome, the locales path is set as "locales/" which potentially conflicts
// with it is in Crosswalk. So the purpose of XWalkResourceDelegate is to set
// the locale pack path of Crosswalk as "locales/xwalk/".
class XWalkResourceDelegate : public ui::ResourceBundle::Delegate {
 public:
  XWalkResourceDelegate();
  ~XWalkResourceDelegate() override;

  // ui:ResourceBundle::Delegate implementation:
  base::FilePath GetPathForResourcePack(
      const base::FilePath& pack_path,
      ui::ScaleFactor scale_factor) override;
  base::FilePath GetPathForLocalePack(
      const base::FilePath& pack_path,
      const std::string& locale) override;
  gfx::Image GetImageNamed(int resource_id) override;
  gfx::Image GetNativeImageNamed(int resource_id) override;
  base::RefCountedStaticMemory* LoadDataResourceBytes(
      int resource_id,
      ui::ScaleFactor scale_factor) override;
  bool GetRawDataResource(int resource_id,
                          ui::ScaleFactor scale_factor,
                          base::StringPiece* value) override;
  bool GetLocalizedString(int message_id, base::string16* value) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkResourceDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_COMMON_XWALK_RESOURCE_DELEGATE_H_
