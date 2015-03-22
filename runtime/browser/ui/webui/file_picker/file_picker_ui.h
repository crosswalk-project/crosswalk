// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_WEBUI_FILE_PICKER_FILE_PICKER_UI_H_
#define XWALK_RUNTIME_BROWSER_UI_WEBUI_FILE_PICKER_FILE_PICKER_UI_H_

#include "base/basictypes.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/web_dialogs/web_dialog_ui.h"

namespace content {
class BrowserContext;
class WebContents;
}

namespace ui {
class WebDialogDelegate;
}

namespace ui {

class FilePickerUI : public WebDialogUI {
 public:
  explicit FilePickerUI(content::WebUI* web_ui);
  ~FilePickerUI() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(FilePickerUI);
};

}  // namespace ui

#endif  // XWALK_RUNTIME_BROWSER_UI_WEBUI_FILE_PICKER_FILE_PICKER_UI_H_
