// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop/download_views.h"

#include <algorithm>

#include "base/strings/utf_string_conversions.h"
#include "grit/xwalk_resources.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "url/gurl.h"
#include "xwalk/runtime/browser/ui/native_app_window_desktop.h"

namespace xwalk {

namespace {

const int kDownloadImageHorizontalPadding = 4;
const int kDownloadImageVerticalPadding = 4;
const int kCloseButtonRightPadding = 4;
const int kCloseButtonLeftPadding = 10;
const int kDownloadItemPadding = 3;

const int kMaxItemTextWidth = 160;
const int kItemTextHorizontalPadding = 3;
const int kItemTextVerticalPadding = 4;

const SkColor kDownloadInProgressColor = SkColorSetARGB(128, 86, 167, 247);
const SkColor kDownloadCompleteColor = SkColorSetARGB(128, 165, 214, 167);
const SkColor kDownloadCancelledColor = SkColorSetARGB(128, 239, 154, 154);

}  // namespace

DownloadBarView::DownloadBarView(NativeAppWindowDesktop* app_view)
    : parent_(app_view),
      download_image_(nullptr),
      ellipsis_label_(nullptr),
      close_button_(nullptr) {
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();

  download_image_ = new views::ImageView();
  download_image_->SetImage(rb.GetImageSkiaNamed(IDR_DOWNLOAD));
  AddChildView(download_image_);

  ellipsis_label_ = new views::Label(base::ASCIIToUTF16("..."));
  AddChildView(ellipsis_label_);

  close_button_ = new views::ImageButton(this);
  close_button_->SetImage(
      views::CustomButton::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_CLOSE));
  close_button_->SetImage(
      views::CustomButton::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_CLOSE_H));
  close_button_->SetImage(
      views::CustomButton::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_CLOSE_P));
  AddChildView(close_button_);
}

DownloadBarView::~DownloadBarView() {
}

void DownloadBarView::Layout() {
  views::View::Layout();

  int x, y, width, height;
  gfx::Size image_size = download_image_->GetPreferredSize();
  width = image_size.width() + 2 * kDownloadImageHorizontalPadding;
  height = image_size.height() + 2 * kDownloadImageVerticalPadding;
  download_image_->SetBounds(kDownloadImageHorizontalPadding,
      kDownloadImageVerticalPadding,
      image_size.width(), image_size.height());

  gfx::Size btn_size = close_button_->GetPreferredSize();
  x = std::max(this->width() - btn_size.width() - kCloseButtonRightPadding, 0);
  y = std::max((height - btn_size.height()) / 2, 0);
  close_button_->SetBounds(x, y, btn_size.width(), btn_size.height());

  int max_item_x = std::max(x - kCloseButtonLeftPadding - kMaxItemTextWidth,
      width);
  x = width;
  for (DownloadItemView* item : download_item_views_) {
    if (x > max_item_x) {
      gfx::Size size = ellipsis_label_->GetPreferredSize();
      ellipsis_label_->SetBounds(x, 0, size.width(), size.height());
      ellipsis_label_->SetVisible(true);
      break;
    }
    gfx::Size item_size = item->GetPreferredSize();
    item->SetBounds(x, 0, item_size.width(), item_size.height());
    item->SetVisible(true);
    x += item_size.width() + kDownloadItemPadding;
  }
}

gfx::Size DownloadBarView::GetPreferredSize() const {
  gfx::Size image_size = download_image_->GetPreferredSize();
  int height = image_size.height() + 2 * kDownloadImageVerticalPadding;
  return gfx::Size(width(), height);
}

void DownloadBarView::OnBoundsChanged(const gfx::Rect& previous_bounds) {
  if (previous_bounds.width() != bounds().width()) {
    ellipsis_label_->SetVisible(false);
    for (DownloadItemView* item : download_item_views_)
      item->SetVisible(false);
  }
}

void DownloadBarView::AddDownloadItem(
    content::DownloadItem* download_item, const base::FilePath& path) {
  DownloadItemView* item_view = new DownloadItemView(download_item, path, this);

  download_item_views_.push_back(item_view);
  AddChildView(item_view);
  SetVisible(true);
  parent_->UpdateWebViewPreferredSize();
  parent_->InvalidateLayout();
  parent_->Layout();
}

void DownloadBarView::ButtonPressed(
    views::Button* button, const ui::Event& event) {
  // Remove completed/failed download items.
  for (auto it = download_item_views_.begin();
      it != download_item_views_.end(); ) {
    if (!(*it)->item() || (*it)->item()->IsDone()) {
      RemoveChildView(*it);
      delete *it;
      it = download_item_views_.erase(it);
    } else {
      ++it;
    }
  }

  SetVisible(false);
  parent()->InvalidateLayout();
  parent()->Layout();
}

class DownloadItemView::DownloadProgressView : public views::Label {
 public:
  DownloadProgressView()
    : progress_(0.0),
      bg_color_(kDownloadInProgressColor) {
  }

  void SetProgress(double progress) {
    DCHECK(progress >= 0 && progress <= 1);
    progress_ = progress;
    SchedulePaint();
  }

  void SetColor(const SkColor color) {
    bg_color_ = color;
    SchedulePaint();
  }

 private:
  void OnPaint(gfx::Canvas* canvas)  override {
    gfx::Rect content_bounds = GetContentsBounds();
    int bar_left = content_bounds.x();
    int bar_top = content_bounds.y();
    int bar_width = content_bounds.width();
    int bar_height = content_bounds.height();

    SkPath path;
    path.addRect(bar_left, bar_top, bar_width * progress_, bar_height);
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setFlags(SkPaint::kAntiAlias_Flag);
    paint.setColor(bg_color_);
    canvas->DrawPath(path, paint);

    views::Label::OnPaint(canvas);
  }

  double progress_;
  SkColor bg_color_;
};

DownloadItemView::DownloadItemView(
    content::DownloadItem* item,
    const base::FilePath& path,
    DownloadBarView* bar_view)
  : progress_view_(nullptr),
    cancel_button_(nullptr),
    parent_(bar_view),
    target_path_(path),
    item_(item) {
  DCHECK(item);
  item->AddObserver(this);

  progress_view_ = new DownloadProgressView();
#if defined (OS_WIN)
  progress_view_->SetText(path.BaseName().value());
#else
  progress_view_->SetText(base::UTF8ToUTF16(path.BaseName().value()));
#endif
  OnDownloadUpdated(item);
  AddChildView(progress_view_);

  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  cancel_button_ = new views::ImageButton(this);
  cancel_button_->SetImage(
      views::CustomButton::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_CLOSE));
  cancel_button_->SetImage(
      views::CustomButton::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_CLOSE_H));
  cancel_button_->SetImage(
      views::CustomButton::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_CLOSE_P));
  AddChildView(cancel_button_);
}

DownloadItemView::~DownloadItemView() {
  if (item_)
    item_->RemoveObserver(this);
}

void DownloadItemView::Layout() {
  views::View::Layout();

  gfx::Size btn_size, text_size;
  if (cancel_button_)
    btn_size = cancel_button_->GetPreferredSize();
  text_size = progress_view_->GetPreferredSize();

  int width, height;
  height = kItemTextVerticalPadding * 2 + text_size.height();
  height = std::max<int>(height, btn_size.height());
  width = kItemTextHorizontalPadding + text_size.width();
  width = std::min<int>(width, kMaxItemTextWidth);
  int y = 0;
  progress_view_->SetBounds(kItemTextHorizontalPadding, y,
                            width - kItemTextHorizontalPadding, height);

  width += kItemTextHorizontalPadding;
  if (cancel_button_) {
    int y = (height - btn_size.height()) / 2;
    cancel_button_->SetBounds(width, y, btn_size.width(), btn_size.height());
    width += btn_size.width();
  }

  preferred_size_ = gfx::Size(width, height);
}

gfx::Size DownloadItemView::GetPreferredSize() const {
  return preferred_size_;
}

void DownloadItemView::OnDownloadUpdated(content::DownloadItem* download) {
  DCHECK_EQ(download, item_);
  progress_view_->SetProgress(0.01 * download->PercentComplete());
  if (item_->IsDone()) {
    RemoveChildView(cancel_button_);
    delete cancel_button_;
    cancel_button_ = nullptr;
    if (item_->GetState() == content::DownloadItem::COMPLETE)
      progress_view_->SetColor(kDownloadCompleteColor);
    else
      progress_view_->SetColor(kDownloadCancelledColor);
    item_->RemoveObserver(this);
    item_ = nullptr;
    parent_->InvalidateLayout();
    parent_->Layout();
  }
}

void DownloadItemView::ButtonPressed(
    views::Button* button, const ui::Event& event) {
  DCHECK(item_);
  item_->Cancel(true);
}

}  // namespace xwalk
