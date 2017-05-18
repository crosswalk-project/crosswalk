// Copyright (c) 2012 Intel Corp
// Copyright (c) 2012 The Chromium Authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell co
// pies of the Software, and to permit persons to whom the Software is furnished
//  to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in al
// l copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM
// PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNES
// S FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
//  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WH
// ETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "cameo/src/browser/ui/native_toolbar_win.h"

#include "base/logging.h"
#include "base/string16.h"
#include "base/utf_string_conversions.h"
#include "cameo/src/browser/shell.h"
#include "grit/cameo_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/layout/box_layout.h"

namespace cameo {

const int kButtonMargin = 2;

NativeToolbarWin::NativeToolbarWin(Shell* shell)
    : shell_(shell) {
}

NativeToolbarWin::~NativeToolbarWin() {
}

views::View* NativeToolbarWin::GetContentsView() {
  return this;
}

void NativeToolbarWin::Layout() {
  int panel_width = width();
  int x = kButtonMargin;

  // Place three left buttons.
  gfx::Size sz = back_button_->GetPreferredSize();
  back_button_->SetBounds(x, (height() - sz.height()) / 2,
                          sz.width(), sz.height());
  x += sz.width() + kButtonMargin;

  sz = forward_button_->GetPreferredSize();
  forward_button_->SetBounds(x, back_button_->y(),
                             sz.width(), sz.height());
  x += sz.width() + kButtonMargin;

  sz = stop_or_refresh_button_->GetPreferredSize();
  stop_or_refresh_button_->SetBounds(x, back_button_->y(),
                                     sz.width(), sz.height());
  x += sz.width() + kButtonMargin;

  // And place dev reload button as far as possible.
  sz = devtools_button_->GetPreferredSize();
  devtools_button_->SetBounds(panel_width - sz.width() - kButtonMargin,
                              back_button_->y(),
                              sz.width(),
                              sz.height());

  // Stretch url entry.
  url_entry_->SetBounds(x,
                        (height() - 24) / 2,
                        devtools_button_->x() - kButtonMargin - x,
                        24);
}

void NativeToolbarWin::ViewHierarchyChanged(bool is_add,
                                                  views::View* parent,
                                                  views::View* child) {
  if (is_add && child == this)
    InitToolbar();
}

void NativeToolbarWin::ContentsChanged(
    views::Textfield* sender,
    const string16& new_contents) {
}

bool NativeToolbarWin::HandleKeyEvent(views::Textfield* sender,
                                      const ui::KeyEvent& key_event) {
  if (key_event.key_code() == ui::VKEY_RETURN) {
    string16 url_string = url_entry_->text();
    if (!url_string.empty()) {
      GURL url(url_string);
      if (!url.has_scheme())
        url = GURL(L"http://" + url_string);
      shell_->LoadURL(url);
    }
  }

  return false;
}

void NativeToolbarWin::ButtonPressed(views::Button* sender,
                                     const ui::Event& event) {
  if (sender == back_button_)
    shell_->GoBackOrForward(-1);
  else if (sender == forward_button_)
    shell_->GoBackOrForward(1);
  // else if (sender == stop_or_refresh_button_)
  //   shell_->ReloadOrStop();
  else if (sender == devtools_button_)
    shell_->ShowDevTools();
  else
    DLOG(INFO) << "Click on unkown toolbar button.";
}

void NativeToolbarWin::InitToolbar() {
  set_background(views::Background::CreateStandardPanelBackground());

  views::BoxLayout* layout = new views::BoxLayout(
      views::BoxLayout::kHorizontal, 5, 5, 10);
  SetLayoutManager(layout);

  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  back_button_ = new views::ImageButton(this);
  back_button_->SetImage(views::CustomButton::STATE_NORMAL,
      rb.GetNativeImageNamed(IDR_CAMEO_BACK).ToImageSkia());
  back_button_->SetImage(views::CustomButton::STATE_HOVERED,
      rb.GetNativeImageNamed(IDR_CAMEO_BACK_H).ToImageSkia());
  back_button_->SetImage(views::CustomButton::STATE_PRESSED,
      rb.GetNativeImageNamed(IDR_CAMEO_BACK_P).ToImageSkia());
  back_button_->SetImage(views::CustomButton::STATE_DISABLED,
      rb.GetNativeImageNamed(IDR_CAMEO_BACK_D).ToImageSkia());
  back_button_->SetAccessibleName(L"Back");
  AddChildView(back_button_);

  forward_button_ = new views::ImageButton(this);
  forward_button_->SetImage(views::CustomButton::STATE_NORMAL,
      rb.GetNativeImageNamed(IDR_CAMEO_FORWARD).ToImageSkia());
  forward_button_->SetImage(views::CustomButton::STATE_HOVERED,
      rb.GetNativeImageNamed(IDR_CAMEO_FORWARD_H).ToImageSkia());
  forward_button_->SetImage(views::CustomButton::STATE_PRESSED,
      rb.GetNativeImageNamed(IDR_CAMEO_FORWARD_P).ToImageSkia());
  forward_button_->SetImage(views::CustomButton::STATE_DISABLED,
      rb.GetNativeImageNamed(IDR_CAMEO_FORWARD_D).ToImageSkia());
  forward_button_->SetAccessibleName(L"Forward");
  AddChildView(forward_button_);

  stop_or_refresh_button_ = new views::ImageButton(this);
  SetIsLoading(true);
  AddChildView(stop_or_refresh_button_);

  url_entry_ = new views::Textfield(views::Textfield::STYLE_DEFAULT);
  url_entry_->SetController(this);
  AddChildView(url_entry_);

  devtools_button_ = new views::ImageButton(this);
  devtools_button_->SetImage(views::CustomButton::STATE_NORMAL,
      rb.GetNativeImageNamed(IDR_CAMEO_TOOLS).ToImageSkia());
  devtools_button_->SetImage(views::CustomButton::STATE_HOVERED,
      rb.GetNativeImageNamed(IDR_CAMEO_TOOLS_H).ToImageSkia());
  devtools_button_->SetImage(views::CustomButton::STATE_PRESSED,
      rb.GetNativeImageNamed(IDR_CAMEO_TOOLS_P).ToImageSkia());
  devtools_button_->SetAccessibleName(L"Devtools");
  AddChildView(devtools_button_);
}

void NativeToolbarWin::SetButtonEnabled(
    NativeAppWindow::ButtonType button, bool enabled) {
  switch (button) {
    case NativeAppWindow::BUTTON_TYPE_BACK:
      back_button_->SetEnabled(enabled);
      break;
    case NativeAppWindow::BUTTON_TYPE_FORWARD:
      forward_button_->SetEnabled(enabled);
      break;
    case NativeAppWindow::BUTTON_TYPE_REFRESH_OR_STOP:
      stop_or_refresh_button_->SetEnabled(enabled);
      break;
    case NativeAppWindow::BUTTON_TYPE_DEVTOOLS:
      devtools_button_->SetEnabled(enabled);
      break;
    default:
      NOTREACHED() << "Invalid button type.";
      break;
  }
}

void NativeToolbarWin::SetUrlEntry(const std::string& url) {
  url_entry_->SetText(UTF8ToUTF16(url));
}

void NativeToolbarWin::SetIsLoading(bool loading) {
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();

  if (loading) {
    stop_or_refresh_button_->SetImage(views::CustomButton::STATE_NORMAL,
        rb.GetNativeImageNamed(IDR_CAMEO_STOP).ToImageSkia());
    stop_or_refresh_button_->SetImage(views::CustomButton::STATE_HOVERED,
        rb.GetNativeImageNamed(IDR_CAMEO_STOP_H).ToImageSkia());
    stop_or_refresh_button_->SetImage(views::CustomButton::STATE_PRESSED,
        rb.GetNativeImageNamed(IDR_CAMEO_STOP_P).ToImageSkia());
    stop_or_refresh_button_->SetImage(views::CustomButton::STATE_DISABLED,
        rb.GetNativeImageNamed(IDR_CAMEO_STOP_D).ToImageSkia());
    stop_or_refresh_button_->SetAccessibleName(L"Stop");
  } else {
    stop_or_refresh_button_->SetImage(views::CustomButton::STATE_NORMAL,
        rb.GetNativeImageNamed(IDR_CAMEO_RELOAD).ToImageSkia());
    stop_or_refresh_button_->SetImage(views::CustomButton::STATE_HOVERED,
        rb.GetNativeImageNamed(IDR_CAMEO_RELOAD_H).ToImageSkia());
    stop_or_refresh_button_->SetImage(views::CustomButton::STATE_PRESSED,
        rb.GetNativeImageNamed(IDR_CAMEO_RELOAD_P).ToImageSkia());
    stop_or_refresh_button_->SetImage(views::CustomButton::STATE_DISABLED,
        rb.GetNativeImageNamed(IDR_CAMEO_RELOAD_D).ToImageSkia());
    stop_or_refresh_button_->SetAccessibleName(L"Reload");
  }

  // Force refresh
  stop_or_refresh_button_->SchedulePaint();
}

}  // namespace cameo
