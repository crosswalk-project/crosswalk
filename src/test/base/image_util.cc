// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/test/base/image_util.h"

#if defined(TOOLKIT_GTK)
#include <gtk/gtk.h>
#endif

#include <string>

#include "base/file_util.h"
#include "base/string_util.h"
#include "ui/gfx/image/image_skia.h"

#if defined(OS_WIN)
#include "ui/gfx/icon_util.h"
#endif

namespace cameo_test_utils {

gfx::Image LoadImageFromFilePath(const base::FilePath& filename) {
  const base::FilePath::StringType kPNGFormat(FILE_PATH_LITERAL(".png"));
  const base::FilePath::StringType kICOFormat(FILE_PATH_LITERAL(".ico"));

  if (EndsWith(filename.value(), kPNGFormat, false)) {
    std::string contents;
    file_util::ReadFileToString(filename, &contents);
    return gfx::Image::CreateFrom1xPNGBytes(
        reinterpret_cast<const unsigned char*>(contents.data()),
            contents.size());
  }

  if (EndsWith(filename.value(), kICOFormat, false)) {
#if defined(OS_WIN)
    HICON icon = static_cast<HICON>(LoadImage(NULL,
                                    filename.value().c_str(),
                                    IMAGE_ICON,
                                    0,
                                    0,
                                    LR_LOADTRANSPARENT | LR_LOADFROMFILE));
    if (icon == NULL)
      return gfx::Image();

    gfx::Image image;
    scoped_ptr<SkBitmap> bitmap(IconUtil::CreateSkBitmapFromHICON(icon));
    if (bitmap.get()) {
      gfx::ImageSkia image_skia = gfx::ImageSkia::CreateFrom1xBitmap(*bitmap);
      image = gfx::Image(image_skia);
    }
    DestroyIcon(icon);

    return image;
#elif defined(TOOLKIT_GTK)
    GdkPixbuf* pixbuf =
        gdk_pixbuf_new_from_file(filename.value().c_str(), NULL);

    if (pixbuf == NULL)
      return gfx::Image();

    return gfx::Image(pixbuf);
#else
  NOTREACHED();
  return gfx::Image();
#endif
  }

  LOG(INFO) << "Only support png and ico file format.";
  return gfx::Image();
}

}  // namespace cameo_test_utils
