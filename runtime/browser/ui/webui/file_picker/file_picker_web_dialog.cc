// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/webui/file_picker/file_picker_web_dialog.h"

#include "base/bind.h"
#include "base/values.h"
#include "base/json/json_writer.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "ui/base/l10n/l10n_util.h"
#include "xwalk/grit/xwalk_resources.h"
#include "xwalk/runtime/browser/ui/browser_dialogs.h"
#include "xwalk/runtime/common/url_constants.h"

using content::WebContents;
using content::WebUIMessageHandler;

namespace {

// Default width/height of the dialog.
const int kDefaultWidth = 350;
const int kDefaultHeight = 225;
}

namespace ui {

class FilePickerMessageHandler : public content::WebUIMessageHandler {
 public:
  FilePickerMessageHandler(
      const ui::FilePickerWebDialog* dialog);
  ~FilePickerMessageHandler() override;
  void RegisterMessages() override;

 private:
  // content::WebUIMessageHandler implementation.
  void OnCancelButtonClicked(const base::ListValue* args);
  void OnSaveButtonClicked(const base::ListValue* args);

  // Weak ptr to parent dialog.
  const ui::FilePickerWebDialog* dialog_;
};

FilePickerMessageHandler::FilePickerMessageHandler(
    const ui::FilePickerWebDialog* dialog)
  : dialog_(dialog) {
}

FilePickerMessageHandler::~FilePickerMessageHandler() {
}

void FilePickerMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "cancel",
      base::Bind(&FilePickerMessageHandler::OnCancelButtonClicked,
                 base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "done",
      base::Bind(&FilePickerMessageHandler::OnSaveButtonClicked,
                 base::Unretained(this)));
}

void FilePickerMessageHandler::OnCancelButtonClicked(
    const base::ListValue* args) {
    std::string file_path;
    dialog_->Close(file_path);
}

void FilePickerMessageHandler::OnSaveButtonClicked(
    const base::ListValue* args) {
    std::string file_path;
    args->GetString(0, &file_path);

    dialog_->Close(file_path);
}

}  // namespace ui

namespace ui {

// static
void FilePickerWebDialog::ShowDialog(SelectFileDialog::Type type,
    gfx::NativeWindow owning_window, content::WebContents* contents,
    SelectFileDialog::Listener* listener) {
  xwalk::ShowWebDialog(owning_window, contents->GetBrowserContext(),
      new FilePickerWebDialog(type, listener));
}

void FilePickerWebDialog::Close(const std::string& file_path) const {
  if (!file_path.empty())
    listener_->FileSelected(base::FilePath(file_path), 0 , NULL);
  else
    listener_->FileSelectionCanceled(NULL);
}

FilePickerWebDialog::FilePickerWebDialog(SelectFileDialog::Type type,
    SelectFileDialog::Listener* listener)
    : type_(type), listener_(listener) {
}

ui::ModalType FilePickerWebDialog::GetDialogModalType() const {
  return ui::MODAL_TYPE_SYSTEM;
}

base::string16 FilePickerWebDialog::GetDialogTitle() const {
  if (type_ == SelectFileDialog::SELECT_OPEN_FILE)
    return l10n_util::GetStringUTF16(IDS_FILE_BROWSER_SELECT_OPEN_FILE_TITLE);
  else if (type_ == SelectFileDialog::SELECT_SAVEAS_FILE)
    return l10n_util::GetStringUTF16(IDS_FILE_BROWSER_SELECT_SAVEAS_FILE_TITLE);

  return base::string16();
}

GURL FilePickerWebDialog::GetDialogContentURL() const {
  return GURL(xwalk::kChromeUIFilePickerURL);
}

void FilePickerWebDialog::GetWebUIMessageHandlers(
     std::vector<content::WebUIMessageHandler*>* handlers) const {
  handlers->push_back(new FilePickerMessageHandler(this));
}

void FilePickerWebDialog::GetDialogSize(gfx::Size* size) const {
  size->SetSize(kDefaultWidth, kDefaultHeight);
}

std::string FilePickerWebDialog::GetDialogArgs() const {
  std::string data;
  base::DictionaryValue file_info;
  if (type_ == SelectFileDialog::SELECT_OPEN_FILE)
    file_info.SetBoolean("promptForOpenFile",  true);
  else
    file_info.SetBoolean("promptForOpenFile",  false);

  file_info.SetString("filePath",  "/home/app");
  base::JSONWriter::Write(&file_info, &data);
  return data;
}

void FilePickerWebDialog::OnDialogClosed(const std::string& json_retval) {
  delete this;
}

void FilePickerWebDialog::OnCloseContents(WebContents* source,
                                                bool* out_close_dialog) {
  if (out_close_dialog)
    *out_close_dialog = true;
}

bool FilePickerWebDialog::ShouldShowDialogTitle() const {
  return true;
}

bool FilePickerWebDialog::HandleContextMenu(
    const content::ContextMenuParams& params) {
  // Disable context menu.
  return true;
}

}  // namespace ui
