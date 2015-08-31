// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_DESKTOP_DOWNLOAD_VIEWS_H_
#define XWALK_RUNTIME_BROWSER_UI_DESKTOP_DOWNLOAD_VIEWS_H_

#include <vector>

#include "content/public/browser/download_item.h"
#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"

namespace xwalk {

class DownloadItemView;
class NativeAppWindow;
class NativeAppWindowDesktop;

class DownloadBarView : public views::ButtonListener,
                        public views::View {
 public:
  explicit DownloadBarView(NativeAppWindowDesktop* app_view);
  ~DownloadBarView() override;

  void AddDownloadItem(content::DownloadItem* download_item,
                       const base::FilePath& path);

  // views::View methods.
  void Layout() override;
  gfx::Size GetPreferredSize() const override;
  void OnBoundsChanged(const gfx::Rect& previous_bounds) override;

  // views::ButtonListener methods.
  void ButtonPressed(views::Button* button, const ui::Event& event) override;

 private:
  NativeAppWindowDesktop* parent_;
  views::ImageView* download_image_;
  views::Label* ellipsis_label_;
  views::ImageButton* close_button_;
  std::vector<DownloadItemView*> download_item_views_;
};

class DownloadItemView : public views::View,
                         public views::ButtonListener,
                         public content::DownloadItem::Observer {
 public:
  DownloadItemView(content::DownloadItem* download_item,
                   const base::FilePath& path,
                   DownloadBarView* download_bar);
  ~DownloadItemView() override;

  const content::DownloadItem* item() const {
    return item_;
  }

  // views::View methods.
  void Layout() override;
  gfx::Size GetPreferredSize() const override;

  // DownloadItem::Observer methods
  void OnDownloadUpdated(content::DownloadItem* download) override;

  // views::ButtonListener methods.
  void ButtonPressed(views::Button* button, const ui::Event& event) override;

 private:
  class DownloadProgressView;

  DownloadProgressView* progress_view_;
  views::ImageButton* cancel_button_;
  DownloadBarView* parent_;
  gfx::Size preferred_size_;

  base::FilePath target_path_;
  content::DownloadItem* item_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_DESKTOP_DOWNLOAD_VIEWS_H_
