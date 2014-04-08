// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_PERMISSION_DIALOG_H_
#define XWALK_APPLICATION_COMMON_PERMISSION_DIALOG_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"
#include "ui/base/ui_base_types.h"
#include "ui/events/event.h"
#include "ui/views/controls/button/checkbox.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/layout_constants.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/dialog_delegate.h"
#include "xwalk/application/common/permission_types.h"
#include "xwalk/application/browser/application_storage_impl.h"

namespace xwalk {
namespace application {
// A custom scrollable view implementation for the dialog.
class CustomScrollableView : public views::View {
 public:
  CustomScrollableView();
  virtual ~CustomScrollableView();

 private:
  virtual void Layout() OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(CustomScrollableView);
};

class PermissionDialog : public views::DialogDelegateView,
                         public views::ButtonListener {
 public:
  PermissionDialog(const std::string& app_id, const std::string& permission);
  virtual ~PermissionDialog();
  virtual bool Cancel() OVERRIDE;
  virtual bool Accept() OVERRIDE;

  // DialogDelegateView overrides:
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual base::string16 GetWindowTitle() const OVERRIDE;
  virtual base::string16 GetDialogButtonLabel(
      ui::DialogButton button) const OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void Layout() OVERRIDE;

  // ButtonListener override:
  virtual void ButtonPressed(views::Button* sender,
      const ui::Event& event) OVERRIDE;

  void SetShowDialogCallback(const PermissionCallback& callback);

 private:
  void Init();
  PromptType GetPolicyEffect(const std::string& permission);

  std::string app_id_;
  base::string16 title_;
  base::string16 request_permission_;
  PermissionCallback show_dialog_callback_;
  views::Label* permission_item_label_;
  views::Label* permission_header_label_;
  views::Checkbox* prompt_checkbox_;
  views::ScrollView* scroll_view_;
  // The container view for the scroll view.
  CustomScrollableView* scrollable_;
  // The preferred size of the dialog.
  gfx::Size dialog_size_;
  PromptType prompt_type_;

  DISALLOW_COPY_AND_ASSIGN(PermissionDialog);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_PERMISSION_DIALOG_H_
