// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/gtk/select_file_dialog_gtk2.h"

#include <string>

#include "base/files/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_thread.h"
#include "ui/aura/window.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/strings/grit/ui_strings.h"
#include "ui/shell_dialogs/select_file_policy.h"

using content::BrowserThread;

namespace {

// Makes sure that .jpg also shows .JPG.
gboolean FileFilterCaseInsensitive(const GtkFileFilterInfo* file_info,
                                   std::string* file_extension) {
  return EndsWith(file_info->filename, *file_extension, false);
}

// Deletes |data| when gtk_file_filter_add_custom() is done with it.
void OnFileFilterDataDestroyed(std::string* file_extension) {
  delete file_extension;
}

const char kAuraTransientParent[] = "aura-transient-parent";

void SetGtkTransientForAura(GtkWidget* dialog, aura::Window* parent) {
  if (!parent || !parent->GetHost())
    return;

  gtk_widget_realize(dialog);

  // We also set the |parent| as a property of |dialog|, so that we can unlink
  // the two later.
  g_object_set_data(G_OBJECT(dialog), kAuraTransientParent, parent);
}

aura::Window* GetAuraTransientParent(GtkWidget* dialog) {
  return reinterpret_cast<aura::Window*>(
      g_object_get_data(G_OBJECT(dialog), kAuraTransientParent));
}

void ClearAuraTransientParent(GtkWidget* dialog) {
  g_object_set_data(G_OBJECT(dialog), kAuraTransientParent, NULL);
}

// The size of the preview we display for selected image files. We set height
// larger than width because generally there is more free space vertically
// than horiztonally (setting the preview image will alway expand the width of
// the dialog, but usually not the height). The image's aspect ratio will always
// be preserved.
static const int kPreviewWidth = 256;
static const int kPreviewHeight = 512;

}  // namespace

namespace xwalk {

base::FilePath* SelectFileDialogGTK2::last_saved_path_ = NULL;
base::FilePath* SelectFileDialogGTK2::last_opened_path_ = NULL;

// static
ui::SelectFileDialog* SelectFileDialogGTK2::Create(
    ui::SelectFileDialog::Listener* listener,
    ui::SelectFilePolicy* policy) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  return new SelectFileDialogGTK2(listener, policy);
}

SelectFileDialogGTK2::SelectFileDialogGTK2(Listener* listener,
                                           ui::SelectFilePolicy* policy)
    : SelectFileDialog(listener, policy),
      file_type_index_(0),
      type_(SELECT_NONE),
      preview_(NULL) {
  if (!last_saved_path_) {
    last_saved_path_ = new base::FilePath();
    last_opened_path_ = new base::FilePath();
  }
}

SelectFileDialogGTK2::~SelectFileDialogGTK2() {
  for (std::set<aura::Window*>::iterator iter = parents_.begin();
       iter != parents_.end(); ++iter) {
    (*iter)->RemoveObserver(this);
  }
  while (dialogs_.begin() != dialogs_.end()) {
    gtk_widget_destroy(*(dialogs_.begin()));
  }
}

void SelectFileDialogGTK2::ListenerDestroyed() {
  listener_ = NULL;
}

bool SelectFileDialogGTK2::IsRunning(gfx::NativeWindow parent_window) const {
  return parents_.find(parent_window) != parents_.end();
}

bool SelectFileDialogGTK2::HasMultipleFileTypeChoicesImpl() {
  return file_types_.extensions.size() > 1;
}

void SelectFileDialogGTK2::OnWindowDestroying(aura::Window* window) {
  // Remove the |parent| property associated with the |dialog|.
  for (std::set<GtkWidget*>::iterator it = dialogs_.begin();
       it != dialogs_.end(); ++it) {
    aura::Window* parent = GetAuraTransientParent(*it);
    if (parent == window)
      ClearAuraTransientParent(*it);
  }

  std::set<aura::Window*>::iterator iter = parents_.find(window);
  if (iter != parents_.end()) {
    (*iter)->RemoveObserver(this);
    parents_.erase(iter);
  }
}

// We ignore |default_extension|.
void SelectFileDialogGTK2::SelectFileImpl(
    Type type,
    const base::string16& title,
    const base::FilePath& default_path,
    const FileTypeInfo* file_types,
    int file_type_index,
    const base::FilePath::StringType& default_extension,
    gfx::NativeWindow owning_window,
    void* params) {
  type_ = type;
  // |owning_window| can be null when user right-clicks on a downloadable item
  // and chooses 'Open Link in New Tab' when 'Ask where to save each file
  // before downloading.' preference is turned on. (http://crbug.com/29213)
  if (owning_window) {
    owning_window->AddObserver(this);
    parents_.insert(owning_window);
  }

  std::string title_string = base::UTF16ToUTF8(title);

  file_type_index_ = file_type_index;
  if (file_types)
    file_types_ = *file_types;

  GtkWidget* dialog = NULL;
  switch (type) {
    case SELECT_FOLDER:
    case SELECT_UPLOAD_FOLDER:
      dialog = CreateSelectFolderDialog(type, title_string, default_path,
                                        owning_window);
      break;
    case SELECT_OPEN_FILE:
      dialog = CreateFileOpenDialog(title_string, default_path, owning_window);
      break;
    case SELECT_OPEN_MULTI_FILE:
      dialog = CreateMultiFileOpenDialog(title_string, default_path,
                                         owning_window);
      break;
    case SELECT_SAVEAS_FILE:
      dialog = CreateSaveAsDialog(title_string, default_path, owning_window);
      break;
    default:
      NOTREACHED();
      return;
  }
  g_signal_connect(dialog, "delete-event",
                   G_CALLBACK(gtk_widget_hide_on_delete), NULL);
  dialogs_.insert(dialog);

  preview_ = gtk_image_new();
  g_signal_connect(dialog, "destroy",
                   G_CALLBACK(OnFileChooserDestroyThunk), this);
  g_signal_connect(dialog, "update-preview",
                   G_CALLBACK(OnUpdatePreviewThunk), this);
  gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview_);

  params_map_[dialog] = params;

  gtk_widget_show_all(dialog);
  gtk_window_set_keep_above(GTK_WINDOW(dialog), true);
}

bool SelectFileDialogGTK2::CallDirectoryExistsOnUIThread(
    const base::FilePath& path) {
  base::ThreadRestrictions::ScopedAllowIO allow_io;
  return base::DirectoryExists(path);
}

void SelectFileDialogGTK2::AddFilters(GtkFileChooser* chooser) {
  for (size_t i = 0; i < file_types_.extensions.size(); ++i) {
    GtkFileFilter* filter = NULL;
    std::set<std::string> fallback_labels;

    for (size_t j = 0; j < file_types_.extensions[i].size(); ++j) {
      const std::string& current_extension = file_types_.extensions[i][j];
      if (!current_extension.empty()) {
        if (!filter)
          filter = gtk_file_filter_new();
        scoped_ptr<std::string> file_extension(
            new std::string("." + current_extension));
        fallback_labels.insert(std::string("*").append(*file_extension));
        gtk_file_filter_add_custom(
            filter,
            GTK_FILE_FILTER_FILENAME,
            reinterpret_cast<GtkFileFilterFunc>(FileFilterCaseInsensitive),
            file_extension.release(),
            reinterpret_cast<GDestroyNotify>(OnFileFilterDataDestroyed));
      }
    }
    // We didn't find any non-empty extensions to filter on.
    if (!filter)
      continue;

    // The description vector may be blank, in which case we are supposed to
    // use some sort of default description based on the filter.
    if (i < file_types_.extension_description_overrides.size()) {
      gtk_file_filter_set_name(filter, base::UTF16ToUTF8(
          file_types_.extension_description_overrides[i]).c_str());
    } else {
      // There is no system default filter description so we use
      // the extensions themselves if the description is blank.
      std::vector<std::string> fallback_labels_vector(fallback_labels.begin(),
                                                      fallback_labels.end());
      std::string fallback_label = JoinString(fallback_labels_vector, ',');
      gtk_file_filter_set_name(filter, fallback_label.c_str());
    }

    gtk_file_chooser_add_filter(chooser, filter);
    if (i == file_type_index_ - 1)
      gtk_file_chooser_set_filter(chooser, filter);
  }

  // Add the *.* filter, but only if we have added other filters (otherwise it
  // is implied).
  if (file_types_.include_all_files && !file_types_.extensions.empty()) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_filter_set_name(filter,
        l10n_util::GetStringUTF8(IDS_SAVEAS_ALL_FILES).c_str());
    gtk_file_chooser_add_filter(chooser, filter);
  }
}

void SelectFileDialogGTK2::FileSelected(GtkWidget* dialog,
                                           const base::FilePath& path) {
  if (type_ == SELECT_SAVEAS_FILE) {
    *last_saved_path_ = path.DirName();
  } else if (type_ == SELECT_OPEN_FILE || type_ == SELECT_FOLDER ||
             type_ == SELECT_UPLOAD_FOLDER) {
    *last_opened_path_ = path.DirName();
  } else {
    NOTREACHED();
  }

  if (listener_) {
    GtkFileFilter* selected_filter =
        gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
    GSList* filters = gtk_file_chooser_list_filters(GTK_FILE_CHOOSER(dialog));
    int idx = g_slist_index(filters, selected_filter);
    g_slist_free(filters);
    listener_->FileSelected(path, idx + 1, PopParamsForDialog(dialog));
  }
  gtk_widget_destroy(dialog);
}

void SelectFileDialogGTK2::MultiFilesSelected(GtkWidget* dialog,
    const std::vector<base::FilePath>& files) {
    *last_opened_path_ = files[0].DirName();

  if (listener_)
    listener_->MultiFilesSelected(files, PopParamsForDialog(dialog));
  gtk_widget_destroy(dialog);
}

void SelectFileDialogGTK2::FileNotSelected(GtkWidget* dialog) {
  void* params = PopParamsForDialog(dialog);
  if (listener_)
    listener_->FileSelectionCanceled(params);
  gtk_widget_destroy(dialog);
}

GtkWidget* SelectFileDialogGTK2::CreateFileOpenHelper(
    const std::string& title,
    const base::FilePath& default_path,
    gfx::NativeWindow parent) {
  GtkWidget* dialog =
      gtk_file_chooser_dialog_new(title.c_str(), NULL,
                                  GTK_FILE_CHOOSER_ACTION_OPEN,
                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                  NULL);
  SetGtkTransientForAura(dialog, parent);

  AddFilters(GTK_FILE_CHOOSER(dialog));
  if (!default_path.empty()) {
    if (CallDirectoryExistsOnUIThread(default_path)) {
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                          default_path.value().c_str());
    } else {
      // If the file doesn't exist, this will just switch to the correct
      // directory. That's good enough.
      gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),
                                    default_path.value().c_str());
    }
  } else if (!last_opened_path_->empty()) {
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                        last_opened_path_->value().c_str());
  }
  return dialog;
}

GtkWidget* SelectFileDialogGTK2::CreateSelectFolderDialog(
    Type type,
    const std::string& title,
    const base::FilePath& default_path,
    gfx::NativeWindow parent) {
  std::string title_string = title;
  if (title_string.empty()) {
    title_string = (type == SELECT_UPLOAD_FOLDER) ?
        l10n_util::GetStringUTF8(IDS_SELECT_UPLOAD_FOLDER_DIALOG_TITLE) :
        l10n_util::GetStringUTF8(IDS_SELECT_FOLDER_DIALOG_TITLE);
  }
  std::string accept_button_label = (type == SELECT_UPLOAD_FOLDER) ?
      l10n_util::GetStringUTF8(IDS_SELECT_UPLOAD_FOLDER_DIALOG_UPLOAD_BUTTON) :
      GTK_STOCK_OPEN;

  GtkWidget* dialog =
      gtk_file_chooser_dialog_new(title_string.c_str(), NULL,
                                  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                  accept_button_label.c_str(),
                                  GTK_RESPONSE_ACCEPT,
                                  NULL);
  SetGtkTransientForAura(dialog, parent);

  if (!default_path.empty()) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),
                                  default_path.value().c_str());
  } else if (!last_opened_path_->empty()) {
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                        last_opened_path_->value().c_str());
  }
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
  g_signal_connect(dialog, "response",
                   G_CALLBACK(OnSelectSingleFolderDialogResponseThunk), this);
  return dialog;
}

GtkWidget* SelectFileDialogGTK2::CreateFileOpenDialog(
    const std::string& title,
    const base::FilePath& default_path,
    gfx::NativeWindow parent) {
  std::string title_string = !title.empty() ? title :
        l10n_util::GetStringUTF8(IDS_OPEN_FILE_DIALOG_TITLE);

  GtkWidget* dialog = CreateFileOpenHelper(title_string, default_path, parent);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
  g_signal_connect(dialog, "response",
                   G_CALLBACK(OnSelectSingleFileDialogResponseThunk), this);
  return dialog;
}

GtkWidget* SelectFileDialogGTK2::CreateMultiFileOpenDialog(
    const std::string& title,
    const base::FilePath& default_path,
    gfx::NativeWindow parent) {
  std::string title_string = !title.empty() ? title :
        l10n_util::GetStringUTF8(IDS_OPEN_FILES_DIALOG_TITLE);

  GtkWidget* dialog = CreateFileOpenHelper(title_string, default_path, parent);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
  g_signal_connect(dialog, "response",
                   G_CALLBACK(OnSelectMultiFileDialogResponseThunk), this);
  return dialog;
}

GtkWidget* SelectFileDialogGTK2::CreateSaveAsDialog(const std::string& title,
    const base::FilePath& default_path, gfx::NativeWindow parent) {
  std::string title_string = !title.empty() ? title :
        l10n_util::GetStringUTF8(IDS_SAVE_AS_DIALOG_TITLE);

  GtkWidget* dialog =
      gtk_file_chooser_dialog_new(title_string.c_str(), NULL,
                                  GTK_FILE_CHOOSER_ACTION_SAVE,
                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                  NULL);
  SetGtkTransientForAura(dialog, parent);

  AddFilters(GTK_FILE_CHOOSER(dialog));
  if (!default_path.empty()) {
    // Since the file may not already exist, we use
    // set_current_folder() followed by set_current_name(), as per the
    // recommendation of the GTK docs.
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
        default_path.DirName().value().c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
        default_path.BaseName().value().c_str());
  } else if (!last_saved_path_->empty()) {
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                        last_saved_path_->value().c_str());
  }
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
                                                 TRUE);
  g_signal_connect(dialog, "response",
                   G_CALLBACK(OnSelectSingleFileDialogResponseThunk), this);
  return dialog;
}

void* SelectFileDialogGTK2::PopParamsForDialog(GtkWidget* dialog) {
  std::map<GtkWidget*, void*>::iterator iter = params_map_.find(dialog);
  DCHECK(iter != params_map_.end());
  void* params = iter->second;
  params_map_.erase(iter);
  return params;
}

void SelectFileDialogGTK2::FileDialogDestroyed(GtkWidget* dialog) {
  dialogs_.erase(dialog);

  // Parent may be NULL in a few cases: 1) on shutdown when
  // AllBrowsersClosed() trigger this handler after all the browser
  // windows got destroyed, or 2) when the parent tab has been opened by
  // 'Open Link in New Tab' context menu on a downloadable item and
  // the tab has no content (see the comment in SelectFile as well).
  aura::Window* parent = GetAuraTransientParent(dialog);
  if (!parent)
    return;

  std::set<aura::Window*>::iterator iter = parents_.find(parent);
  if (iter != parents_.end()) {
    (*iter)->RemoveObserver(this);
    parents_.erase(iter);
  } else {
    NOTREACHED();
  }
}

bool SelectFileDialogGTK2::IsCancelResponse(gint response_id) {
  bool is_cancel = response_id == GTK_RESPONSE_CANCEL ||
                   response_id == GTK_RESPONSE_DELETE_EVENT;
  if (is_cancel)
    return true;

  DCHECK(response_id == GTK_RESPONSE_ACCEPT);
  return false;
}

void SelectFileDialogGTK2::SelectSingleFileHelper(GtkWidget* dialog,
    gint response_id,
    bool allow_folder) {
  if (IsCancelResponse(response_id)) {
    FileNotSelected(dialog);
    return;
  }

  gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
  if (!filename) {
    FileNotSelected(dialog);
    return;
  }

  base::FilePath path(filename);
  g_free(filename);

  if (allow_folder) {
    FileSelected(dialog, path);
    return;
  }

  if (CallDirectoryExistsOnUIThread(path))
    FileNotSelected(dialog);
  else
    FileSelected(dialog, path);
}

void SelectFileDialogGTK2::OnSelectSingleFileDialogResponse(
    GtkWidget* dialog, int response_id) {
  SelectSingleFileHelper(dialog, response_id, false);
}

void SelectFileDialogGTK2::OnSelectSingleFolderDialogResponse(
    GtkWidget* dialog, int response_id) {
  SelectSingleFileHelper(dialog, response_id, true);
}

void SelectFileDialogGTK2::OnSelectMultiFileDialogResponse(GtkWidget* dialog,
                                                              int response_id) {
  if (IsCancelResponse(response_id)) {
    FileNotSelected(dialog);
    return;
  }

  GSList* filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
  if (!filenames) {
    FileNotSelected(dialog);
    return;
  }

  std::vector<base::FilePath> filenames_fp;
  for (GSList* iter = filenames; iter != NULL; iter = g_slist_next(iter)) {
    base::FilePath path(static_cast<char*>(iter->data));
    g_free(iter->data);
    if (CallDirectoryExistsOnUIThread(path))
      continue;
    filenames_fp.push_back(path);
  }
  g_slist_free(filenames);

  if (filenames_fp.empty()) {
    FileNotSelected(dialog);
    return;
  }
  MultiFilesSelected(dialog, filenames_fp);
}

void SelectFileDialogGTK2::OnFileChooserDestroy(GtkWidget* dialog) {
  FileDialogDestroyed(dialog);
}

void SelectFileDialogGTK2::OnUpdatePreview(GtkWidget* chooser) {
  gchar* filename = gtk_file_chooser_get_preview_filename(
      GTK_FILE_CHOOSER(chooser));
  if (!filename)
    return;
  // This will preserve the image's aspect ratio.
  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_size(filename, kPreviewWidth,
                                                       kPreviewHeight, NULL);
  g_free(filename);
  if (pixbuf) {
    gtk_image_set_from_pixbuf(GTK_IMAGE(preview_), pixbuf);
    g_object_unref(pixbuf);
  }
  gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(chooser),
                                             pixbuf ? TRUE : FALSE);
}

}  // namespace xwalk
