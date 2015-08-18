// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_desktop.h"

#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "grit/xwalk_strings.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/gfx/canvas.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "xwalk/runtime/browser/ui/top_view_layout_views.h"

namespace xwalk {

class NativeAppWindowDesktop::AddressView : public views::Label {
 public:
  AddressView()
    : progress_(0) {
  }
  ~AddressView() override {}

  void SetProgress(double progress) {
    DCHECK(progress >= 0 && progress <= 1);
    progress_ = progress;
    SchedulePaint();
  }

 private:
  void OnPaint(gfx::Canvas* canvas) override {
    static const SkColor bg_color = SkColorSetARGB(128, 86, 167, 247);
    gfx::Rect content_bounds = GetContentsBounds();
    int bar_left = content_bounds.x();
    int bar_top = content_bounds.y();
    int bar_width = content_bounds.width();
    int bar_height = content_bounds.height();

    SkPath path;
    path.addRect(bar_left, bar_top + 2, bar_width * progress_, bar_height - 2);
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setFlags(SkPaint::kAntiAlias_Flag);
    paint.setColor(bg_color);
    canvas->DrawPath(path, paint);

    views::Label::OnPaint(canvas);
  }

  double progress_;
};

NativeAppWindowDesktop::NativeAppWindowDesktop(
    const NativeAppWindow::CreateParams& create_params)
    : NativeAppWindowViews(create_params),
      toolbar_view_(nullptr),
      back_button_(nullptr),
      forward_button_(nullptr),
      refresh_button_(nullptr),
      stop_button_(nullptr),
      address_bar_(nullptr),
      contents_view_(nullptr) {
}

NativeAppWindowDesktop::~NativeAppWindowDesktop() {
}

void NativeAppWindowDesktop::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this) {
    if (create_params().mode == blink::WebDisplayModeMinimalUi)
      InitMinimalUI();
    else
      InitStandaloneUI();
  }
}

void NativeAppWindowDesktop::OnBoundsChanged(const gfx::Rect& previous_bounds) {
  UpdateWebViewPreferredSize();
}

void NativeAppWindowDesktop::InitStandaloneUI() {
  TopViewLayout* layout = new TopViewLayout();
  SetLayoutManager(layout);

  web_view_ = new views::WebView(NULL);
  web_view_->SetWebContents(web_contents_);
  AddChildView(web_view_);
  layout->set_content_view(web_view_);
}

void NativeAppWindowDesktop::InitMinimalUI() {
  set_background(views::Background::CreateStandardPanelBackground());
  views::BoxLayout* box_layout =
      new views::BoxLayout(views::BoxLayout::kVertical, 0, 0, 0);
  box_layout->SetDefaultFlex(1);
  SetLayoutManager(box_layout);

  views::View* container = new views::View();
  views::GridLayout* layout = new views::GridLayout(container);
  container->SetLayoutManager(layout);
  AddChildView(container);

  views::ColumnSet* column_set = layout->AddColumnSet(0);
  column_set->AddPaddingColumn(0, 2);
  column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1,
                        views::GridLayout::USE_PREF, 0, 0);
  column_set->AddPaddingColumn(0, 2);

  layout->AddPaddingRow(0, 2);

  // Add toolbar buttons and URL address
  {
    layout->StartRow(0, 0);
    toolbar_view_ = new views::View;
    views::GridLayout* toolbar_layout = new views::GridLayout(toolbar_view_);
    toolbar_view_->SetLayoutManager(toolbar_layout);

    views::ColumnSet* toolbar_column_set = toolbar_layout->AddColumnSet(0);
    // Back button
    back_button_ = new views::LabelButton(
        this, l10n_util::GetStringUTF16(IDS_MINIMAL_UI_BACK));
    back_button_->SetStyle(views::Button::STYLE_BUTTON);
    gfx::Size back_button_size = back_button_->GetPreferredSize();
    toolbar_column_set->AddColumn(
        views::GridLayout::CENTER, views::GridLayout::CENTER, 0,
        views::GridLayout::FIXED, back_button_size.width(),
        back_button_size.width() / 2);
    // Forward button
    forward_button_ = new views::LabelButton(
        this, l10n_util::GetStringUTF16(IDS_MINIMAL_UI_FORWARD));
    forward_button_->SetStyle(views::Button::STYLE_BUTTON);
    gfx::Size forward_button_size = forward_button_->GetPreferredSize();
    toolbar_column_set->AddColumn(
        views::GridLayout::CENTER, views::GridLayout::CENTER, 0,
        views::GridLayout::FIXED, forward_button_size.width(),
        forward_button_size.width() / 2);
    // Refresh button
    refresh_button_ = new views::LabelButton(
        this, l10n_util::GetStringUTF16(IDS_MINIMAL_UI_REFRESH));
    refresh_button_->SetStyle(views::Button::STYLE_BUTTON);
    gfx::Size refresh_button_size = refresh_button_->GetPreferredSize();
    toolbar_column_set->AddColumn(
        views::GridLayout::CENTER, views::GridLayout::CENTER, 0,
        views::GridLayout::FIXED, refresh_button_size.width(),
        refresh_button_size.width() / 2);
    // Stop button
    stop_button_ = new views::LabelButton(
        this, l10n_util::GetStringUTF16(IDS_MINIMAL_UI_STOP));
    stop_button_->SetStyle(views::Button::STYLE_BUTTON);
    gfx::Size stop_button_size = stop_button_->GetPreferredSize();
    toolbar_column_set->AddColumn(
        views::GridLayout::CENTER, views::GridLayout::CENTER, 0,
        views::GridLayout::FIXED, stop_button_size.width(),
        stop_button_size.width() / 2);
    toolbar_column_set->AddPaddingColumn(0, 2);
    // URL entry
    address_bar_ = new AddressView();
    toolbar_column_set->AddColumn(views::GridLayout::FILL,
                                  views::GridLayout::FILL, 1,
                                  views::GridLayout::USE_PREF, 0, 0);
    toolbar_column_set->AddPaddingColumn(0, 2);

    // Fill up the first row
    toolbar_layout->StartRow(0, 0);
    toolbar_layout->AddView(back_button_);
    toolbar_layout->AddView(forward_button_);
    toolbar_layout->AddView(refresh_button_);
    toolbar_layout->AddView(stop_button_);
    toolbar_layout->AddView(address_bar_);

    layout->AddView(toolbar_view_);
  }

  layout->AddPaddingRow(0, 5);

  // Add web contents view as the second row
  {
    contents_view_ = new views::View;
    contents_view_->SetLayoutManager(new views::FillLayout());

    web_view_ = new views::WebView(web_contents_->GetBrowserContext());
    web_view_->SetWebContents(web_contents_);
    web_contents_->Focus();
    contents_view_->AddChildView(web_view_);
    layout->StartRow(1, 0);
    layout->AddView(contents_view_);

    address_bar_->SetText(
        base::ASCIIToUTF16(web_contents_->GetVisibleURL().spec()));
  }

  layout->AddPaddingRow(0, 3);
}

void NativeAppWindowDesktop::ButtonPressed(views::Button* sender,
                                           const ui::Event& event) {
  if (sender == back_button_)
    delegate_->OnBackPressed();
  else if (sender == forward_button_)
    delegate_->OnForwardPressed();
  else if (sender == refresh_button_)
    delegate_->OnReloadPressed();
  else if (sender == stop_button_)
    delegate_->OnStopPressed();
  else
    NOTREACHED();
}

void NativeAppWindowDesktop::SetLoadProgress(double progress) {
  if (toolbar_view_) {
    DCHECK(address_bar_);
    address_bar_->SetProgress(progress);
  }
}

void NativeAppWindowDesktop::SetAddressURL(const std::string& url) {
  if (toolbar_view_) {
    DCHECK(address_bar_);
    address_bar_->SetText(base::ASCIIToUTF16(url));
  }
}

void NativeAppWindowDesktop::UpdateWebViewPreferredSize() {
  int height = this->height();
  if (toolbar_view_) {
    height -= toolbar_view_->GetPreferredSize().height();
    // Minus vertical paddings.
    height = height - 2 - 5 - 3;
  }

  views::View* download_bar = GetViewByID(VIEW_ID_DOWNLOAD_BAR);
  if (download_bar)
    height -= download_bar->GetPreferredSize().height();

  web_view_->SetPreferredSize(gfx::Size(width(), height));
}

}  // namespace xwalk
