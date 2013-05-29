// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/ui/gtk/javascript_app_modal_dialog_gtk.h"

#include <gtk/gtk.h>

#include "base/logging.h"
#include "base/utf_string_conversions.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/javascript_app_modal_dialog.h"
#include "cameo/src/runtime/browser/ui/gtk/gtk_util.h"
#include "grit/cameo_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/ui_base_types.h"

namespace {

// We stash pointers to widgets on the gtk_dialog so we can refer to them
// after dialog creation.
const char kPromptTextId[] = "runtime_prompt_text";
const char kSuppressCheckboxId[] = "runtime_suppress_checkbox";

// If there's a text entry in the dialog, get the text from the first one and
// return it.
string16 GetPromptText(GtkDialog* dialog) {
  GtkWidget* widget = static_cast<GtkWidget*>(
      g_object_get_data(G_OBJECT(dialog), kPromptTextId));
  if (widget)
    return UTF8ToUTF16(gtk_entry_get_text(GTK_ENTRY(widget)));
  return string16();
}

// If there's a toggle button in the dialog, return the toggled state.
// Otherwise, return false.
bool ShouldSuppressJSDialogs(GtkDialog* dialog) {
  GtkWidget* widget = static_cast<GtkWidget*>(
      g_object_get_data(G_OBJECT(dialog), kSuppressCheckboxId));
  if (widget)
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  return false;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// JavaScriptAppModalDialogGtk, public:

JavaScriptAppModalDialogGtk::JavaScriptAppModalDialogGtk(
    JavaScriptAppModalDialog* dialog,
    gfx::NativeWindow parent_window)
    : dialog_(dialog) {
  GtkButtonsType buttons = GTK_BUTTONS_NONE;
  GtkMessageType message_type = GTK_MESSAGE_OTHER;

  // We add in the OK button manually later because we want to focus it
  // explicitly.
  switch (dialog_->javascript_message_type()) {
    case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
      buttons = GTK_BUTTONS_NONE;
      message_type = GTK_MESSAGE_WARNING;
      break;

    case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
      if (dialog_->is_before_unload_dialog()) {
        // onbeforeunload also uses a confirm prompt, it just has custom
        // buttons.  We add the buttons using gtk_dialog_add_button below.
        buttons = GTK_BUTTONS_NONE;
      } else {
        buttons = GTK_BUTTONS_CANCEL;
      }
      message_type = GTK_MESSAGE_QUESTION;
      break;

    case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT:
      buttons = GTK_BUTTONS_CANCEL;
      message_type = GTK_MESSAGE_QUESTION;
      break;

    default:
      NOTREACHED();
  }

  gtk_dialog_ = gtk_message_dialog_new(parent_window,
      GTK_DIALOG_MODAL, message_type, buttons, "%s",
      UTF16ToUTF8(dialog_->message_text()).c_str());
  g_signal_connect(gtk_dialog_, "delete-event",
                   G_CALLBACK(gtk_widget_hide_on_delete), NULL);
  gtk_util::ApplyMessageDialogQuirks(gtk_dialog_);
  gtk_window_set_title(GTK_WINDOW(gtk_dialog_),
                       UTF16ToUTF8(dialog_->title()).c_str());

  // Adjust content area as needed.  Set up the prompt text entry or
  // suppression check box.
  if (dialog_->javascript_message_type() ==
          content::JAVASCRIPT_MESSAGE_TYPE_PROMPT) {
    GtkWidget* content_area =
        gtk_dialog_get_content_area(GTK_DIALOG(gtk_dialog_));
    GtkWidget* text_box = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(text_box),
        UTF16ToUTF8(dialog_->default_prompt_text()).c_str());
    gtk_box_pack_start(GTK_BOX(content_area), text_box, TRUE, TRUE, 0);
    g_object_set_data(G_OBJECT(gtk_dialog_), kPromptTextId, text_box);
    gtk_entry_set_activates_default(GTK_ENTRY(text_box), TRUE);
  }

  if (dialog_->display_suppress_checkbox()) {
    GtkWidget* content_area =
        gtk_dialog_get_content_area(GTK_DIALOG(gtk_dialog_));
    GtkWidget* check_box = gtk_check_button_new_with_label(
        l10n_util::GetStringUTF8(
        IDS_JAVASCRIPT_MESSAGEBOX_SUPPRESS_OPTION).c_str());
    gtk_box_pack_start(GTK_BOX(content_area), check_box, TRUE, TRUE, 0);
    g_object_set_data(G_OBJECT(gtk_dialog_), kSuppressCheckboxId, check_box);
  }

  // Adjust buttons/action area as needed.
  if (dialog_->is_before_unload_dialog()) {
    std::string button_text = l10n_util::GetStringUTF8(
        dialog_->is_reload() ?
        IDS_BEFORERELOAD_MESSAGEBOX_OK_BUTTON_LABEL :
        IDS_BEFOREUNLOAD_MESSAGEBOX_OK_BUTTON_LABEL);
    gtk_dialog_add_button(GTK_DIALOG(gtk_dialog_), button_text.c_str(),
        GTK_RESPONSE_OK);

    button_text = l10n_util::GetStringUTF8(
        dialog_->is_reload() ?
        IDS_BEFORERELOAD_MESSAGEBOX_CANCEL_BUTTON_LABEL :
        IDS_BEFOREUNLOAD_MESSAGEBOX_CANCEL_BUTTON_LABEL);
    gtk_dialog_add_button(GTK_DIALOG(gtk_dialog_), button_text.c_str(),
        GTK_RESPONSE_CANCEL);
  } else {
    // Add the OK button and focus it.
    GtkWidget* ok_button = gtk_dialog_add_button(GTK_DIALOG(gtk_dialog_),
        GTK_STOCK_OK, GTK_RESPONSE_OK);
    if (dialog_->javascript_message_type() !=
            content::JAVASCRIPT_MESSAGE_TYPE_PROMPT)
      gtk_widget_grab_focus(ok_button);
  }

  gtk_dialog_set_default_response(GTK_DIALOG(gtk_dialog_), GTK_RESPONSE_OK);
  g_signal_connect(gtk_dialog_, "response", G_CALLBACK(OnResponseThunk), this);
}

JavaScriptAppModalDialogGtk::~JavaScriptAppModalDialogGtk() {
}

////////////////////////////////////////////////////////////////////////////////
// JavaScriptAppModalDialogGtk, NativeAppModalDialog implementation:

int JavaScriptAppModalDialogGtk::GetAppModalDialogButtons() const {
  switch (dialog_->javascript_message_type()) {
    case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
      return ui::DIALOG_BUTTON_OK;

    case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
      return ui::DIALOG_BUTTON_OK | ui::DIALOG_BUTTON_CANCEL;

    case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT:
      return ui::DIALOG_BUTTON_OK;

    default:
      NOTREACHED();
      return 0;
  }
}

void JavaScriptAppModalDialogGtk::ShowAppModalDialog() {
  gtk_util::ShowDialogWithMinLocalizedWidth(GTK_WIDGET(gtk_dialog_),
      IDS_ALERT_DIALOG_WIDTH_CHARS);
}

void JavaScriptAppModalDialogGtk::ActivateAppModalDialog() {
  DCHECK(gtk_dialog_);
  gtk_window_present(GTK_WINDOW(gtk_dialog_));
}

void JavaScriptAppModalDialogGtk::CloseAppModalDialog() {
  DCHECK(gtk_dialog_);
  OnResponse(gtk_dialog_, GTK_RESPONSE_DELETE_EVENT);
}

void JavaScriptAppModalDialogGtk::AcceptAppModalDialog() {
  OnResponse(gtk_dialog_, GTK_RESPONSE_OK);
}

void JavaScriptAppModalDialogGtk::CancelAppModalDialog() {
  OnResponse(gtk_dialog_, GTK_RESPONSE_CANCEL);
}

////////////////////////////////////////////////////////////////////////////////
// JavaScriptAppModalDialogGtk, private:

void JavaScriptAppModalDialogGtk::OnResponse(GtkWidget* dialog,
                                             int response_id) {
  switch (response_id) {
    case GTK_RESPONSE_OK:
      // The first arg is the prompt text and the second is true if we want to
      // suppress additional popups from the page.
      dialog_->OnAccept(GetPromptText(GTK_DIALOG(dialog)),
                        ShouldSuppressJSDialogs(GTK_DIALOG(dialog)));
      break;

    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_DELETE_EVENT:   // User hit the X on the dialog.
      dialog_->OnCancel(ShouldSuppressJSDialogs(GTK_DIALOG(dialog)));
      break;

    default:
      NOTREACHED();
  }
  gtk_widget_destroy(dialog);

  delete this;
}

////////////////////////////////////////////////////////////////////////////////
// NativeAppModalDialog, public:

// static
NativeAppModalDialog* NativeAppModalDialog::CreateNativeJavaScriptPrompt(
    JavaScriptAppModalDialog* dialog,
    gfx::NativeWindow parent_window) {
  return new JavaScriptAppModalDialogGtk(dialog, parent_window);
}
