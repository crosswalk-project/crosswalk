// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell_download_manager_delegate.h"

#if defined(TOOLKIT_GTK)
#include <gtk/gtk.h>
#endif

#if defined(OS_WIN)
#include <windows.h>
#include <commdlg.h>
#endif

#include <string>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "net/base/net_util.h"

namespace cameo {

ShellDownloadManagerDelegate::ShellDownloadManagerDelegate()
    : download_manager_(NULL),
      suppress_prompting_(false) {
  // Balanced in Shutdown();
  AddRef();
}

ShellDownloadManagerDelegate::~ShellDownloadManagerDelegate() {
}


void ShellDownloadManagerDelegate::SetDownloadManager(
    content::DownloadManager* download_manager) {
  download_manager_ = download_manager;
}

void ShellDownloadManagerDelegate::Shutdown() {
  Release();
}

bool ShellDownloadManagerDelegate::DetermineDownloadTarget(
    content::DownloadItem* download,
    const content::DownloadTargetCallback& callback) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  // This assignment needs to be here because even at the call to
  // SetDownloadManager, the system is not fully initialized.
  if (default_download_path_.empty()) {
    default_download_path_ = download_manager_->GetBrowserContext()->GetPath().
        Append(FILE_PATH_LITERAL("Downloads"));
  }

  if (!download->GetForcedFilePath().empty()) {
    callback.Run(download->GetForcedFilePath(),
                 content::DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                 content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                 download->GetForcedFilePath());
    return true;
  }

  base::FilePath generated_name = net::GenerateFileName(
      download->GetURL(),
      download->GetContentDisposition(),
      EmptyString(),
      download->GetSuggestedFilename(),
      download->GetMimeType(),
      "download");

  content::BrowserThread::PostTask(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(
          &ShellDownloadManagerDelegate::GenerateFilename,
          this, download->GetId(), callback, generated_name,
          default_download_path_));
  return true;
}

bool ShellDownloadManagerDelegate::ShouldOpenDownload(
      content::DownloadItem* item,
      const content::DownloadOpenDelayedCallback& callback) {
  return true;
}

void ShellDownloadManagerDelegate::GenerateFilename(
    int32 download_id,
    const content::DownloadTargetCallback& callback,
    const base::FilePath& generated_name,
    const base::FilePath& suggested_directory) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));
  if (!file_util::PathExists(suggested_directory))
    file_util::CreateDirectory(suggested_directory);

  base::FilePath suggested_path(suggested_directory.Append(generated_name));
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &ShellDownloadManagerDelegate::OnDownloadPathGenerated,
          this, download_id, callback, suggested_path));
}

void ShellDownloadManagerDelegate::OnDownloadPathGenerated(
    int32 download_id,
    const content::DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  if (suppress_prompting_) {
    // Testing exit.
    callback.Run(suggested_path,
                 content::DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                 content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                 suggested_path.AddExtension(FILE_PATH_LITERAL(".crdownload")));
    return;
  }

  ChooseDownloadPath(download_id, callback, suggested_path);
}

void ShellDownloadManagerDelegate::ChooseDownloadPath(
    int32 download_id,
    const content::DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  content::DownloadItem* item = download_manager_->GetDownload(download_id);
  if (!item || (item->GetState() != content::DownloadItem::IN_PROGRESS))
    return;

  base::FilePath result;
#if defined(OS_WIN) && !defined(USE_AURA)
  std::wstring file_part = base::FilePath(suggested_path).BaseName().value();
  wchar_t file_name[MAX_PATH];
  base::wcslcpy(file_name, file_part.c_str(), arraysize(file_name));
  OPENFILENAME save_as;
  ZeroMemory(&save_as, sizeof(save_as));
  save_as.lStructSize = sizeof(OPENFILENAME);
  save_as.hwndOwner = item->GetWebContents()->GetView()->GetNativeView();
  save_as.lpstrFile = file_name;
  save_as.nMaxFile = arraysize(file_name);

  std::wstring directory;
  if (!suggested_path.empty())
    directory = suggested_path.DirName().value();

  save_as.lpstrInitialDir = directory.c_str();
  save_as.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLESIZING |
                  OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;

  if (GetSaveFileName(&save_as))
    result = base::FilePath(std::wstring(save_as.lpstrFile));
#elif defined(TOOLKIT_GTK)
  GtkWidget* dialog;
  gfx::NativeWindow parent_window;
  std::string base_name = base::FilePath(suggested_path).BaseName().value();

  parent_window = item->GetWebContents()->GetView()->GetTopLevelNativeWindow();
  dialog = gtk_file_chooser_dialog_new("Save File",
                                       parent_window,
                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                       NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
                                                 TRUE);
  gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
                                    base_name.c_str());

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char* filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    result = base::FilePath(filename);
  }
  gtk_widget_destroy(dialog);
#else
  NOTIMPLEMENTED();
#endif

  callback.Run(result, content::DownloadItem::TARGET_DISPOSITION_PROMPT,
               content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS, result);
}

void ShellDownloadManagerDelegate::SetDownloadBehaviorForTesting(
    const base::FilePath& default_download_path) {
  default_download_path_ = default_download_path;
  suppress_prompting_ = true;
}

}  // namespace cameo
