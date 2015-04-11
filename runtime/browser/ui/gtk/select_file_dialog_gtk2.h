// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_GTK_SELECT_FILE_DIALOG_GTK2_H_
#define XWALK_RUNTIME_BROWSER_UI_GTK_SELECT_FILE_DIALOG_GTK2_H_

#include <gtk/gtk.h>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/strings/string_util.h"
#include "ui/aura/window_observer.h"
#include "ui/base/glib/glib_signal.h"
#include "ui/shell_dialogs/select_file_dialog.h"

namespace aura {
class Window;
}  // namespace aura

namespace base {
class FilePath;
}  // namespace base

namespace ui {
class SelectFilePolicy;
}  // namespace ui

namespace xwalk {

typedef struct _GtkWidget GtkWidget;

// Macros CHROMEG_CALLBACK* come from "ui/base/glib/glib_signal.h".
#define CHROMEGTK_CALLBACK_0(CLASS, RETURN, METHOD) \
  CHROMEG_CALLBACK_0(CLASS, RETURN, METHOD, GtkWidget*);

#define CHROMEGTK_CALLBACK_1(CLASS, RETURN, METHOD, ARG1) \
  CHROMEG_CALLBACK_1(CLASS, RETURN, METHOD, GtkWidget*, ARG1);

// Implementation of SelectFileDialog that shows a Gtk common dialog for
// choosing a file or folder. This acts as a modal dialog.
class SelectFileDialogGTK2 : public ui::SelectFileDialog,
                             public aura::WindowObserver {
 public:
  // Main factory method which returns correct type.
  static ui::SelectFileDialog* Create(Listener* listener,
                                      ui::SelectFilePolicy* policy);

 protected:
  explicit SelectFileDialogGTK2(Listener* listener,
                                   ui::SelectFilePolicy* policy);
  ~SelectFileDialogGTK2() override;

  // BaseShellDialog implementation.
  void ListenerDestroyed() override;
  bool IsRunning(gfx::NativeWindow parent_window) const override;

  // SelectFileDialog implementation.
  // |params| is user data we pass back via the Listener interface.
  void SelectFileImpl(Type type,
                      const base::string16& title,
                      const base::FilePath& default_path,
                      const FileTypeInfo* file_types,
                      int file_type_index,
                      const base::FilePath::StringType& default_extension,
                      gfx::NativeWindow owning_window,
                      void* params) override;

  // Wrapper for base::DirectoryExists() that allow access on the UI
  // thread. Use this only in the file dialog functions, where it's ok
  // because the file dialog has to do many stats anyway. One more won't
  // hurt too badly and it's likely already cached.
  bool CallDirectoryExistsOnUIThread(const base::FilePath& path);

  // The file filters.
  FileTypeInfo file_types_;

  // The index of the default selected file filter.
  // Note: This starts from 1, not 0.
  size_t file_type_index_;

  // The type of dialog we are showing the user.
  Type type_;

  // These two variables track where the user last saved a file or opened a
  // file so that we can display future dialogs with the same starting path.
  static base::FilePath* last_saved_path_;
  static base::FilePath* last_opened_path_;

 private:
  bool HasMultipleFileTypeChoicesImpl() override;

  // Overridden from aura::WindowObserver.
  void OnWindowDestroying(aura::Window* window) override;

  // Add the filters from |file_types_| to |chooser|.
  void AddFilters(GtkFileChooser* chooser);

  // Notifies the listener that a single file was chosen.
  void FileSelected(GtkWidget* dialog, const base::FilePath& path);

  // Notifies the listener that multiple files were chosen.
  void MultiFilesSelected(GtkWidget* dialog,
                          const std::vector<base::FilePath>& files);

  // Notifies the listener that no file was chosen (the action was canceled).
  // Dialog is passed so we can find that |params| pointer that was passed to
  // us when we were told to show the dialog.
  void FileNotSelected(GtkWidget* dialog);

  GtkWidget* CreateSelectFolderDialog(
      Type type,
      const std::string& title,
      const base::FilePath& default_path,
      gfx::NativeWindow parent);

  GtkWidget* CreateFileOpenDialog(const std::string& title,
      const base::FilePath& default_path, gfx::NativeWindow parent);

  GtkWidget* CreateMultiFileOpenDialog(const std::string& title,
      const base::FilePath& default_path, gfx::NativeWindow parent);

  GtkWidget* CreateSaveAsDialog(const std::string& title,
      const base::FilePath& default_path, gfx::NativeWindow parent);

  // Removes and returns the |params| associated with |dialog| from
  // |params_map_|.
  void* PopParamsForDialog(GtkWidget* dialog);

  // Take care of internal data structures when a file dialog is destroyed.
  void FileDialogDestroyed(GtkWidget* dialog);

  // Check whether response_id corresponds to the user cancelling/closing the
  // dialog. Used as a helper for the below callbacks.
  bool IsCancelResponse(gint response_id);

  // Common function for OnSelectSingleFileDialogResponse and
  // OnSelectSingleFolderDialogResponse.
  void SelectSingleFileHelper(GtkWidget* dialog,
                              gint response_id,
                              bool allow_folder);

  // Common function for CreateFileOpenDialog and CreateMultiFileOpenDialog.
  GtkWidget* CreateFileOpenHelper(const std::string& title,
                                  const base::FilePath& default_path,
                                  gfx::NativeWindow parent);

  // Callback for when the user responds to a Save As or Open File dialog.
  CHROMEGTK_CALLBACK_1(SelectFileDialogGTK2, void,
                       OnSelectSingleFileDialogResponse, int);

  // Callback for when the user responds to a Select Folder dialog.
  CHROMEGTK_CALLBACK_1(SelectFileDialogGTK2, void,
                       OnSelectSingleFolderDialogResponse, int);

  // Callback for when the user responds to a Open Multiple Files dialog.
  CHROMEGTK_CALLBACK_1(SelectFileDialogGTK2, void,
                       OnSelectMultiFileDialogResponse, int);

  // Callback for when the file chooser gets destroyed.
  CHROMEGTK_CALLBACK_0(SelectFileDialogGTK2, void, OnFileChooserDestroy);

  // Callback for when we update the preview for the selection.
  CHROMEGTK_CALLBACK_0(SelectFileDialogGTK2, void, OnUpdatePreview);

  // A map from dialog windows to the |params| user data associated with them.
  std::map<GtkWidget*, void*> params_map_;

  // The GtkImage widget for showing previews of selected images.
  GtkWidget* preview_;

  // All our dialogs.
  std::set<GtkWidget*> dialogs_;

  // The set of all parent windows for which we are currently running dialogs.
  std::set<aura::Window*> parents_;

  DISALLOW_COPY_AND_ASSIGN(SelectFileDialogGTK2);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_GTK_SELECT_FILE_DIALOG_GTK2_H_

