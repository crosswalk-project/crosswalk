// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/permission_dialog.h"

namespace xwalk {
namespace application {

const int kDialogHeight = 200;
const int kDialogWidth = 300;
// Width of the left column of the dialog when the extension requests
// permissions.
const int kPermissionsLeftColumnWidth = 250;

CustomScrollableView::CustomScrollableView() {}
CustomScrollableView::~CustomScrollableView() {}

void CustomScrollableView::Layout() {
  SetBounds(x(), y(), width(), GetHeightForWidth(width()));
  views::View::Layout();
}

PermissionDialog::PermissionDialog(
    const std::string& app_id, const std::string& permission) {
  LOG(INFO) << "Xu in PermissionDialog::PermissionDialog()";
  app_id_ = app_id;
  request_permission_ = base::ASCIIToUTF16(permission);
  prompt_type_ = GetPolicyEffect(permission);
  Init();
}

PermissionDialog::~PermissionDialog() {
}

bool PermissionDialog::Cancel() {
  LOG(INFO) << "Xu clicked Cancel";
  show_dialog_callback_.Run(DENY_ALWAYS);
  return true;
}

bool PermissionDialog::Accept() {
  LOG(INFO) << "Xu clicked OK";
  show_dialog_callback_.Run(ALLOW_ALWAYS);
  return true;
}

gfx::Size PermissionDialog::GetPreferredSize() {
  return dialog_size_;
}

base::string16 PermissionDialog::GetWindowTitle() const {
  return title_;
}

void PermissionDialog::Layout() {
  scroll_view_->SetBounds(0, 0, width(), height());
  DialogDelegateView::Layout();
}

void PermissionDialog::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), SK_ColorWHITE);
}

void PermissionDialog::ButtonPressed(
    views::Button* sender, const ui::Event& event) {
  LOG(INFO) << "Xu enter ButtonPressed";
  // TODO(Xu): Update permission value of application if needed
  NOTREACHED();
  return;
}

void PermissionDialog::SetShowDialogCallback(
    const PermissionCallback& callback) {
  LOG(INFO) << "Xu in PermissionDialog::set_callback";
  show_dialog_callback_ = callback;
}

base::string16 PermissionDialog::GetDialogButtonLabel(
    ui::DialogButton button) const {
  if (button == ui::DIALOG_BUTTON_OK)
    return base::ASCIIToUTF16("Permit");
  if (button == ui::DIALOG_BUTTON_CANCEL) {
    return base::ASCIIToUTF16("Deny");
  }
  NOTREACHED();
  return base::string16();
}

void PermissionDialog::Init() {
  LOG(INFO) << "in permisisonDialog Init";
  scroll_view_ = new views::ScrollView();
  scroll_view_->set_hide_horizontal_scrollbar(true);
  AddChildView(scroll_view_);
  scrollable_ = new CustomScrollableView();
  scroll_view_->SetContents(scrollable_);
  views::GridLayout* layout = views::GridLayout::CreatePanel(scrollable_);
  scrollable_->SetLayoutManager(layout);

  int column_set_id = 0;
  views::ColumnSet* column_set = layout->AddColumnSet(column_set_id);

  column_set->AddColumn(views::GridLayout::LEADING,
      views::GridLayout::FILL,
      0,  // no resizing
      views::GridLayout::USE_PREF,
      0,  // no fixed width
      kPermissionsLeftColumnWidth);
  layout->AddPaddingRow(0, views::kRelatedControlVerticalSpacing);
  layout->StartRow(0, 0);
  permission_header_label_ =
    new views::Label(base::ASCIIToUTF16("Widget requires access to:"));
  permission_header_label_->SetMultiLine(true);
  permission_header_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  permission_header_label_->SizeToFit(0);
  layout->AddView(permission_header_label_);

  layout->AddPaddingRow(0, views::kRelatedControlVerticalSpacing);
  layout->StartRow(0, 0);
  permission_item_label_ = new views::Label(request_permission_);
  permission_item_label_->SetMultiLine(true);
  permission_item_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  permission_item_label_->SizeToFit(0);
  layout->AddView(permission_item_label_);

  if (prompt_type_ == PROMPT_SESSION || prompt_type_ == PROMPT_BLANKET) {
    layout->AddPaddingRow(0, views::kRelatedControlVerticalSpacing);
    layout->StartRow(0, 0);
    switch (prompt_type_) {
      case PROMPT_SESSION:
        prompt_checkbox_ = new views::Checkbox(
            base::ASCIIToUTF16("Remember for one run"));
        break;
      case PROMPT_BLANKET:
        prompt_checkbox_ = new views::Checkbox(
            base::ASCIIToUTF16("Keep setting as permanent"));
        break;
      default:
        break;
    }
    layout->AddView(prompt_checkbox_);
  }

  gfx::Size scrollable_size = scrollable_->GetPreferredSize();
  scrollable_->SetBoundsRect(gfx::Rect(scrollable_size));
  dialog_size_ = gfx::Size(kDialogWidth, kDialogHeight);
}

PromptType PermissionDialog::GetPolicyEffect(const std::string& permission) {
  // TODO(Xu): Get policy effect from system policy, currently just for
  // testing, return PROMPT_SESSION
  return PROMPT_SESSION;
}

}  // namespace application
}  // namespace xwalk
