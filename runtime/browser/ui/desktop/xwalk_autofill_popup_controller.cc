// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop/xwalk_autofill_popup_controller.h"

#include <algorithm>
#include <vector>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "components/autofill/core/browser/autofill_popup_delegate.h"
#include "components/autofill/core/browser/popup_item_ids.h"
#include "components/autofill/core/browser/suggestion.h"
#include "components/autofill/core/common/autofill_switches.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "grit/components_scaled_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/display/screen.h"
#include "ui/events/event.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/geometry/vector2d.h"
#include "ui/gfx/text_elider.h"
#include "ui/gfx/text_utils.h"
#include "xwalk/runtime/browser/ui/desktop/xwalk_autofill_popup_view.h"
#include "xwalk/runtime/browser/ui/desktop/xwalk_popup_controller.h"

using base::WeakPtr;

namespace xwalk {

// Used to indicate that no line is currently selected by the user.
const int kNoSelection = -1;

// The vertical height of each row in pixels.
const size_t kRowHeight = 24;

// The vertical height of a separator in pixels.
const size_t kSeparatorHeight = 1;

// Size difference between name and label in pixels.
const int kLabelFontSizeDelta = -2;

struct DataResource {
  const char* name;
  int id;
};

const DataResource kDataResources[] = {
  { "americanExpressCC", IDR_AUTOFILL_CC_AMEX },
  { "dinersCC", IDR_AUTOFILL_CC_GENERIC },
  { "discoverCC", IDR_AUTOFILL_CC_DISCOVER },
  { "genericCC", IDR_AUTOFILL_CC_GENERIC },
  { "jcbCC", IDR_AUTOFILL_CC_GENERIC },
  { "masterCardCC", IDR_AUTOFILL_CC_MASTERCARD },
  { "visaCC", IDR_AUTOFILL_CC_VISA },
#if defined(OS_MACOSX) && !defined(OS_IOS)
  { "macContactsIcon", IDR_AUTOFILL_MAC_CONTACTS_ICON },
#endif
};

// static
WeakPtr<XWalkAutofillPopupController> XWalkAutofillPopupController::GetOrCreate(
    WeakPtr<XWalkAutofillPopupController> previous,
    WeakPtr<autofill::AutofillPopupDelegate> delegate,
    content::WebContents* web_contents,
    gfx::NativeView container_view,
    const gfx::RectF& element_bounds,
    base::i18n::TextDirection text_direction) {
  if (previous && previous->web_contents() == web_contents &&
    previous->delegate_.get() == delegate.get() &&
    previous->container_view() == container_view &&
    previous->element_bounds() == element_bounds) {
    previous->ClearState();
    return previous;
  }

  if (previous)
    previous->Hide();

  XWalkAutofillPopupController* controller =
    new XWalkAutofillPopupController(
    delegate, web_contents, container_view, element_bounds,
    text_direction);
  return controller->GetWeakPtr();
}

XWalkAutofillPopupController::XWalkAutofillPopupController(
    base::WeakPtr<autofill::AutofillPopupDelegate> delegate,
    content::WebContents* web_contents,
    gfx::NativeView container_view,
    const gfx::RectF& element_bounds,
    base::i18n::TextDirection text_direction)
    : popup_controller_(new XWalkPopupController(element_bounds,
                                                text_direction,
                                                container_view,
                                                web_contents)),
    view_(nullptr),
    delegate_(delegate),
    weak_ptr_factory_(this) {
  ClearState();
  popup_controller_->SetKeyPressCallback(
      base::Bind(&XWalkAutofillPopupController::HandleKeyPressEvent,
      base::Unretained(this)));
  label_font_list_ = value_font_list_.DeriveWithSizeDelta(kLabelFontSizeDelta);
  title_font_list_ = value_font_list_.DeriveWithWeight(gfx::Font::Weight::BOLD);
#if defined(OS_MACOSX)
  // There is no italic version of the system font.
  warning_font_list_ = value_font_list_;
#else
  warning_font_list_ = value_font_list_.DeriveWithStyle(gfx::Font::ITALIC);
#endif
}

XWalkAutofillPopupController::~XWalkAutofillPopupController() {}

void XWalkAutofillPopupController::Show(
  const std::vector<autofill::Suggestion>& suggestions) {
  SetValues(suggestions);
  DCHECK_EQ(suggestions_.size(), elided_values_.size());
  DCHECK_EQ(suggestions_.size(), elided_labels_.size());

  UpdatePopupBounds();
  int popup_width = popup_bounds().width();

  // Elide the name and label strings so that the popup fits in the available
  // space.
  for (size_t i = 0; i < suggestions_.size(); ++i) {
    int value_width =
        gfx::GetStringWidth(suggestions_[i].value, GetValueFontListForRow(i));
    int label_width =
        gfx::GetStringWidth(suggestions_[i].label, GetLabelFontList());
    int total_text_length = value_width + label_width;

    // The line can have no strings if it represents a UI element, such as
    // a separator line.
    if (total_text_length == 0)
      continue;

    int available_width = popup_width - RowWidthWithoutText(i);

    // Each field receives space in proportion to its length.
    int value_size = available_width * value_width / total_text_length;
    elided_values_[i] = gfx::ElideText(suggestions_[i].value,
        GetValueFontListForRow(i),
        value_size, gfx::ELIDE_TAIL);

    int label_size = available_width * label_width / total_text_length;
    elided_labels_[i] = gfx::ElideText(suggestions_[i].label,
        GetLabelFontList(),
        label_size, gfx::ELIDE_TAIL);
  }

  if (!view_) {
    view_ = XWalkAutofillPopupView::Create(this);

    // It is possible to fail to create the popup, in this case
    // treat the popup as hiding right away.
    if (!view_) {
      Hide();
      return;
    }

    ShowView();
  } else {
    UpdateBoundsAndRedrawPopup();
  }

  popup_controller_->RegisterKeyPressCallback();
  delegate_->OnPopupShown();

  DCHECK_EQ(suggestions_.size(), elided_values_.size());
  DCHECK_EQ(suggestions_.size(), elided_labels_.size());
}

void XWalkAutofillPopupController::Hide() {
  popup_controller_->UnregisterKeyPressCallback();
  if (delegate_)
    delegate_->OnPopupHidden();

  if (view_)
    view_->Hide();

  delete this;
}

void XWalkAutofillPopupController::ViewDestroyed() {
  // The view has already been destroyed so clear the reference to it.
  view_ = nullptr;

  Hide();
}

bool XWalkAutofillPopupController::HandleKeyPressEvent(
    const content::NativeWebKeyboardEvent& event) {
  switch (event.windowsKeyCode) {
  case ui::VKEY_UP:
    SelectPreviousLine();
    return true;
  case ui::VKEY_DOWN:
    SelectNextLine();
    return true;
  case ui::VKEY_PRIOR:  // Page up.
    // Set no line and then select the next line in case the first line is not
    // selectable.
    SetSelectedLine(kNoSelection);
    SelectNextLine();
    return true;
  case ui::VKEY_NEXT:  // Page down.
    SetSelectedLine(GetLineCount() - 1);
    return true;
  case ui::VKEY_ESCAPE:
    Hide();
    return true;
  case ui::VKEY_DELETE:
    return false;
    return (event.modifiers & content::NativeWebKeyboardEvent::ShiftKey) &&
      RemoveSelectedLine();
  case ui::VKEY_TAB:
    // A tab press should cause the selected line to be accepted, but still
    // return false so the tab key press propagates and changes the cursor
    // location.
    AcceptSelectedLine();
    return false;
  case ui::VKEY_RETURN:
    return AcceptSelectedLine();
  default:
    return false;
  }
}

void XWalkAutofillPopupController::UpdateBoundsAndRedrawPopup() {
  // TODO(csharp): Since UpdatePopupBounds can change the position of the popup,
  // the popup could end up jumping from above the element to below it.
  // It is unclear if it is better to keep the popup where it was, or if it
  // should try and move to its desired position.
  UpdatePopupBounds();

  view_->UpdateBoundsAndRedrawPopup();
}

void XWalkAutofillPopupController::SelectionCleared() {
  SetSelectedLine(kNoSelection);
}

void XWalkAutofillPopupController::AcceptSuggestion(size_t index) {
    const autofill::Suggestion& suggestion = suggestions_[index];
  delegate_->DidAcceptSuggestion(suggestion.value, suggestion.frontend_id,
    index);
}

bool XWalkAutofillPopupController::IsWarning(size_t index) const {
  return suggestions_[index].frontend_id ==
      autofill::POPUP_ITEM_ID_WARNING_MESSAGE;
}

int XWalkAutofillPopupController::GetIconResourceID(
    const base::string16& resource_name) const {
  int result = -1;
  for (size_t i = 0; i < arraysize(kDataResources); ++i) {
    if (resource_name == base::ASCIIToUTF16(kDataResources[i].name)) {
      result = kDataResources[i].id;
      break;
    }
  }
  return result;
}

void XWalkAutofillPopupController::SetSelectionAtPoint(
    const gfx::Point& point) {
  SetSelectedLine(LineFromY(point.y()));
}

bool XWalkAutofillPopupController::AcceptSelectedLine() {
  if (selected_line_ == kNoSelection)
    return false;

  DCHECK_GE(selected_line_, 0);
  DCHECK_LT(selected_line_, static_cast<int>(GetLineCount()));

  if (!CanAccept(suggestions_[selected_line_].frontend_id))
    return false;

  AcceptSuggestion(selected_line_);
  return true;
}

bool XWalkAutofillPopupController::RemoveSelectedLine() {
  if (selected_line_ == kNoSelection)
    return false;

  DCHECK_GE(selected_line_, 0);
  DCHECK_LT(selected_line_, static_cast<int>(GetLineCount()));
  return RemoveSuggestion(selected_line_);
}

gfx::Rect XWalkAutofillPopupController::GetRowBounds(size_t index) {
  int top = XWalkAutofillPopupView::kPopupBorderThickness;
  for (size_t i = 0; i < index; ++i) {
    top += GetRowHeightFromId(suggestions_.at(i).frontend_id);
  }

  return gfx::Rect(
      XWalkAutofillPopupView::kPopupBorderThickness,
      top,
      popup_bounds_.width() - 2 *
        XWalkAutofillPopupView::kPopupBorderThickness,
      GetRowHeightFromId(suggestions_.at(index).frontend_id));
}

void XWalkAutofillPopupController::SetPopupBounds(const gfx::Rect& bounds) {
  popup_bounds_ = bounds;
  UpdateBoundsAndRedrawPopup();
}

bool XWalkAutofillPopupController::IsRTL() const {
  return popup_controller_->is_rtl();
}

size_t XWalkAutofillPopupController::GetLineCount() const {
  return suggestions_.size();
}

const autofill::Suggestion& XWalkAutofillPopupController::GetSuggestionAt(
  size_t row) const {
  return suggestions_.at(row);
}

bool XWalkAutofillPopupController::RemoveSuggestion(int list_index) {
  if (!delegate_->RemoveSuggestion(suggestions_.at(list_index).value,
    suggestions_.at(list_index).frontend_id)) {
    return false;
  }

  // Remove the deleted element.
  suggestions_.erase(suggestions_.begin() + list_index);
  elided_values_.erase(elided_values_.begin() + list_index);
  elided_labels_.erase(elided_labels_.begin() + list_index);

  SetSelectedLine(kNoSelection);

  if (HasSuggestions()) {
    delegate_->ClearPreviewedForm();
    UpdateBoundsAndRedrawPopup();
  } else {
    Hide();
  }

  return true;
}

bool XWalkAutofillPopupController::HasSuggestions() {
  if (suggestions_.empty())
    return false;
  int id = suggestions_.at(0).frontend_id;
  return id > 0 ||
      id == autofill::POPUP_ITEM_ID_AUTOCOMPLETE_ENTRY ||
      id == autofill::POPUP_ITEM_ID_PASSWORD_ENTRY ||
      id == autofill::POPUP_ITEM_ID_DATALIST_ENTRY ||
      id == autofill::POPUP_ITEM_ID_SCAN_CREDIT_CARD;
}

const base::string16& XWalkAutofillPopupController::GetElidedValueAt(
  size_t row) const {
  return elided_values_.at(row);
}

const base::string16& XWalkAutofillPopupController::GetElidedLabelAt(
  size_t row) const {
  return elided_labels_.at(row);
}

const gfx::FontList& XWalkAutofillPopupController::GetValueFontListForRow(
  size_t index) const {
  if (suggestions_.at(index).frontend_id ==
      autofill::POPUP_ITEM_ID_WARNING_MESSAGE)
    return warning_font_list_;

  if (suggestions_.at(index).frontend_id == autofill::POPUP_ITEM_ID_TITLE)
    return title_font_list_;

  return value_font_list_;
}

const gfx::FontList& XWalkAutofillPopupController::GetLabelFontList() const {
  return label_font_list_;
}

void XWalkAutofillPopupController::SetSelectedLine(int selected_line) {
  if (selected_line_ == selected_line)
    return;

  if (selected_line_ != kNoSelection &&
    static_cast<size_t>(selected_line_) < suggestions_.size())
    InvalidateRow(selected_line_);

  if (selected_line != kNoSelection) {
    InvalidateRow(selected_line);

    if (!CanAccept(suggestions_[selected_line].frontend_id))
      selected_line = kNoSelection;
  }

  selected_line_ = selected_line;

  if (selected_line_ != kNoSelection) {
    delegate_->DidSelectSuggestion(suggestions_[selected_line_].value,
      suggestions_[selected_line_].frontend_id);
  } else {
    delegate_->ClearPreviewedForm();
  }
}

void XWalkAutofillPopupController::SelectNextLine() {
  int new_selected_line = selected_line_ + 1;

  // Skip over any lines that can't be selected.
  while (static_cast<size_t>(new_selected_line) < GetLineCount() &&
      !CanAccept(suggestions_.at(new_selected_line).frontend_id)) {
    ++new_selected_line;
  }

  if (new_selected_line >= static_cast<int>(GetLineCount()))
    new_selected_line = 0;

  SetSelectedLine(new_selected_line);
}

void XWalkAutofillPopupController::SelectPreviousLine() {
  int new_selected_line = selected_line_ - 1;

  // Skip over any lines that can't be selected.
  while (new_selected_line > kNoSelection &&
      !CanAccept(GetSuggestionAt(new_selected_line).frontend_id)) {
    --new_selected_line;
  }

  if (new_selected_line <= kNoSelection)
    new_selected_line = GetLineCount() - 1;

  SetSelectedLine(new_selected_line);
}

void XWalkAutofillPopupController::ShowView() {
  view_->Show();
}

const gfx::Rect& XWalkAutofillPopupController::popup_bounds() const {
  return popup_bounds_;
}

gfx::NativeView XWalkAutofillPopupController::container_view() {
  return popup_controller_->container_view();
}

const gfx::RectF& XWalkAutofillPopupController::element_bounds() const {
  return popup_controller_->element_bounds();
}

content::WebContents* XWalkAutofillPopupController::web_contents() {
  return popup_controller_->web_contents();
}

void XWalkAutofillPopupController::InvalidateRow(size_t row) {
  DCHECK_LE(static_cast<size_t>(0), row);
  DCHECK(row < suggestions_.size());
  view_->InvalidateRow(row);
}

int XWalkAutofillPopupController::GetDesiredPopupWidth() const {
  int popup_width = popup_controller_->RoundedElementBounds().width();
  for (size_t i = 0; i < GetLineCount(); ++i) {
    int row_size =
        gfx::GetStringWidth(GetElidedValueAt(i), value_font_list_) +
        gfx::GetStringWidth(GetElidedLabelAt(i), label_font_list_) +
        RowWidthWithoutText(i);

    popup_width = std::max(popup_width, row_size);
  }

  return popup_width;
}

int XWalkAutofillPopupController::GetDesiredPopupHeight() const {
  int popup_height = 2 * XWalkAutofillPopupView::kPopupBorderThickness;

  for (size_t i = 0; i < suggestions_.size(); ++i) {
    popup_height += GetRowHeightFromId(suggestions_[i].frontend_id);
  }

  return popup_height;
}

int XWalkAutofillPopupController::RowWidthWithoutText(int row) const {
  int row_size = XWalkAutofillPopupView::kEndPadding;

  if (!elided_labels_.at(row).empty())
    row_size += XWalkAutofillPopupView::kNamePadding;

  // Add the Autofill icon size, if required.
  const base::string16& icon = suggestions_.at(row).icon;
  if (!icon.empty()) {
    int icon_width = ui::ResourceBundle::GetSharedInstance().GetImageNamed(
        GetIconResourceID(icon)).Width();
    row_size += icon_width + XWalkAutofillPopupView::kIconPadding;
  }

  // Add the padding at the end.
  row_size += XWalkAutofillPopupView::kEndPadding;

  // Add room for the popup border.
  row_size += 2 * XWalkAutofillPopupView::kPopupBorderThickness;

  return row_size;
}

void XWalkAutofillPopupController::UpdatePopupBounds() {
  int popup_width = GetDesiredPopupWidth();
  int popup_height = GetDesiredPopupHeight();

  popup_bounds_ = popup_controller_->GetPopupBounds(popup_width, popup_height);
}

WeakPtr<XWalkAutofillPopupController>
    XWalkAutofillPopupController::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

void XWalkAutofillPopupController::ClearState() {
  // Don't clear view_, because otherwise the popup will have
  // to get regenerated and this will cause flickering.

  popup_bounds_ = gfx::Rect();

  suggestions_.clear();
  selected_line_ = kNoSelection;
}

int XWalkAutofillPopupController::LineFromY(int y) {
  int current_height = XWalkAutofillPopupView::kPopupBorderThickness;

  for (size_t i = 0; i < suggestions_.size(); ++i) {
    current_height += GetRowHeightFromId(suggestions_[i].frontend_id);

    if (y <= current_height)
      return i;
  }

  // The y value goes beyond the popup so stop the selection at the last line.
  return GetLineCount() - 1;
}

int XWalkAutofillPopupController::GetRowHeightFromId(int identifier) const {
  if (identifier == autofill::POPUP_ITEM_ID_SEPARATOR)
    return kSeparatorHeight;

  return kRowHeight;
}

bool XWalkAutofillPopupController::CanAccept(int id) {
  return id != autofill::POPUP_ITEM_ID_SEPARATOR &&
         id != autofill::POPUP_ITEM_ID_WARNING_MESSAGE &&
         id != autofill::POPUP_ITEM_ID_TITLE;
}

void XWalkAutofillPopupController::SetValues(
  const std::vector<autofill::Suggestion>& suggestions) {
  suggestions_ = suggestions;
  elided_values_.resize(suggestions.size());
  elided_labels_.resize(suggestions.size());
  for (size_t i = 0; i < suggestions.size(); i++) {
    elided_values_[i] = suggestions[i].value;
    elided_labels_[i] = suggestions[i].label;
  }
}

}  // namespace xwalk
