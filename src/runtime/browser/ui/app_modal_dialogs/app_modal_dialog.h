// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_UI_APP_MODAL_DIALOGS_APP_MODAL_DIALOG_H_
#define CAMEO_SRC_RUNTIME_BROWSER_UI_APP_MODAL_DIALOGS_APP_MODAL_DIALOG_H_

#include <string>

#include "base/basictypes.h"
#include "base/string16.h"
#include "build/build_config.h"

class NativeAppModalDialog;

namespace content {
class WebContents;
}

// A controller+model base class for modal dialogs.
class AppModalDialog {
 public:
  // A union of data necessary to determine the type of message box to
  // show.
  AppModalDialog(content::WebContents* web_contents, const string16& title);
  virtual ~AppModalDialog();

  // Called by the AppModalDialogQueue to show this dialog.
  void ShowModalDialog();

  // Called by the AppModalDialogQueue to activate the dialog.
  void ActivateModalDialog();

  // Closes the dialog if it is showing.
  void CloseModalDialog();

  // Completes dialog handling, shows next modal dialog from the queue.
  void CompleteDialog();

  string16 title() const { return title_; }
  NativeAppModalDialog* native_dialog() const { return native_dialog_; }
  content::WebContents* web_contents() const { return web_contents_; }

  // Creates an implementation of NativeAppModalDialog and shows it.
  // When the native dialog is closed, the implementation of
  // NativeAppModalDialog should call OnAccept or OnCancel to notify the
  // renderer of the user's action. The NativeAppModalDialog is also
  // expected to delete the AppModalDialog associated with it.
  void CreateAndShowDialog();

  // Returns true if the dialog is still valid. As dialogs are created they are
  // added to the AppModalDialogQueue. When the current modal dialog finishes
  // and it's time to show the next dialog in the queue IsValid is invoked.
  // If IsValid returns false the dialog is deleted and not shown.
  bool IsValid();

  // Methods overridable by AppModalDialog subclasses:

  // Invalidates the dialog, therefore causing it to not be shown when its turn
  // to be shown comes around.
  virtual void Invalidate();

  // Used only for testing. Returns whether the dialog is a JavaScript modal
  // dialog.
  virtual bool IsJavaScriptModalDialog();

 protected:
  // Overridden by subclasses to create the feature-specific native dialog box.
  virtual NativeAppModalDialog* CreateNativeDialog() = 0;

  // False if the dialog should no longer be shown, e.g. because the underlying
  // tab navigated away while the dialog was queued.
  bool valid_;

  // The toolkit-specific implementation of the app modal dialog box.
  NativeAppModalDialog* native_dialog_;

 private:
  // Information about the message box is held in the following variables.
  string16 title_;

  content::WebContents* web_contents_;

  // True if CompleteDialog was called.
  bool completed_;

  DISALLOW_COPY_AND_ASSIGN(AppModalDialog);
};

#endif  // CAMEO_SRC_RUNTIMEBROWSER_UI_APP_MODAL_DIALOGS_APP_MODAL_DIALOG_H_
