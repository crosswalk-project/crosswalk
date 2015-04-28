// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/webui/file_picker/file_picker_ui.h"

#include "base/values.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/views/controls/webview/web_dialog_view.h"
#include "ui/views/widget/widget.h"
#include "xwalk/grit/xwalk_resources.h"
#include "xwalk/runtime/browser/ui/webui/xwalk_web_contents_handler.h"
#include "xwalk/runtime/common/url_constants.h"

namespace ui {
FilePickerUI::FilePickerUI(content::WebUI* web_ui)
  : WebDialogUI(web_ui) {
  content::WebUIDataSource* html_source = content::WebUIDataSource::Create(
      xwalk::kChromeUIFilePickerHost);

  html_source->AddLocalizedString("filePickerFileNameLabel",
       IDS_FILE_PICKER_FILE_NAME);

  html_source->AddLocalizedString("saveButtonText", IDS_SAVE);
  html_source->AddLocalizedString("openButtonText", IDS_OK);
  html_source->AddLocalizedString("cancelButtonText", IDS_CANCEL);

  html_source->SetJsonPath("strings.js");

  html_source->AddResourcePath("file_picker.js", IDR_FILE_PICKER_JS);
  html_source->AddResourcePath("file_picker.css", IDR_FILE_PICKER_CSS);
  html_source->SetDefaultResource(IDR_FILE_PICKER_HTML);

  content::BrowserContext* browser_context =
      web_ui->GetWebContents()->GetBrowserContext();
  content::WebUIDataSource::Add(browser_context, html_source);
}

FilePickerUI::~FilePickerUI() {
}

}  // namespace ui
