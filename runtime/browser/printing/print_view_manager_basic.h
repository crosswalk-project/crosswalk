// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_PRINTING_PRINT_VIEW_MANAGER_BASIC_H_
#define XWALK_RUNTIME_BROWSER_PRINTING_PRINT_VIEW_MANAGER_BASIC_H_

#include "base/macros.h"
#include "content/public/browser/web_contents_user_data.h"
#include "xwalk/runtime/browser/printing/print_view_manager_base.h"

namespace xwalk {

// Manages the print commands for a WebContents - basic version.
class PrintViewManagerBasic
    : public PrintViewManagerBase,
      public content::WebContentsUserData<PrintViewManagerBasic> {
 public:
  ~PrintViewManagerBasic() override;

 private:
  explicit PrintViewManagerBasic(content::WebContents* web_contents);
  friend class content::WebContentsUserData<PrintViewManagerBasic>;

  DISALLOW_COPY_AND_ASSIGN(PrintViewManagerBasic);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_PRINTING_PRINT_VIEW_MANAGER_BASIC_H_
