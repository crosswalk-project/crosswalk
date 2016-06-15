// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/top_view_layout_views.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "ui/views/view.h"

using views::View;
using xwalk::TopViewLayout;

namespace {

class TopViewLayoutTest : public testing::Test {
 public:
  void SetUp() override {
    host_.reset(new View);
  }

  std::unique_ptr<View> host_;
  std::unique_ptr<TopViewLayout> layout_;
};

class ViewWithPreferredSize : public View {
 public:
  explicit ViewWithPreferredSize(const gfx::Size& preferred_size)
      : preferred_size_(preferred_size) {}

  void set_preferred_size(const gfx::Size& preferred_size) {
    preferred_size_ = preferred_size;
  }

  // View implementation.
  gfx::Size GetPreferredSize() const override {
    return preferred_size_;
  }

 private:
  gfx::Size preferred_size_;
};

}  // namespace

TEST_F(TopViewLayoutTest, OnlyContentView) {
  layout_.reset(new TopViewLayout);
  View* content = new View();
  host_->AddChildView(content);
  layout_->set_content_view(content);

  host_->SetSize(gfx::Size(200, 150));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 0, 200, 150), content->bounds());

  host_->SetSize(gfx::Size(300, 400));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 0, 300, 400), content->bounds());
}

TEST_F(TopViewLayoutTest, BothViews) {
  layout_.reset(new TopViewLayout);

  View* content = new View();
  host_->AddChildView(content);
  layout_->set_content_view(content);

  View* top_view = new ViewWithPreferredSize(gfx::Size(200, 20));
  host_->AddChildView(top_view);
  layout_->set_top_view(top_view);

  host_->SetSize(gfx::Size(200, 150));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 20, 200, 130), content->bounds());
  EXPECT_EQ(gfx::Rect(0, 0, 200, 20), top_view->bounds());

  host_->SetSize(gfx::Size(300, 400));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 20, 300, 380), content->bounds());
  EXPECT_EQ(gfx::Rect(0, 0, 200, 20), top_view->bounds());
}

TEST_F(TopViewLayoutTest, ChangingTopViewHeight) {
  layout_.reset(new TopViewLayout);

  View* content = new View();
  host_->AddChildView(content);
  layout_->set_content_view(content);

  ViewWithPreferredSize* top_view =
      new ViewWithPreferredSize(gfx::Size(200, 20));
  host_->AddChildView(top_view);
  layout_->set_top_view(top_view);

  host_->SetSize(gfx::Size(200, 150));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 20, 200, 130), content->bounds());
  EXPECT_EQ(gfx::Rect(0, 0, 200, 20), top_view->bounds());

  top_view->set_preferred_size(gfx::Size(100, 100));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 100, 200, 50), content->bounds());
  EXPECT_EQ(gfx::Rect(0, 0, 100, 100), top_view->bounds());

  host_->SetSize(gfx::Size(300, 400));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 100, 300, 300), content->bounds());
  EXPECT_EQ(gfx::Rect(0, 0, 100, 100), top_view->bounds());

  top_view->set_preferred_size(gfx::Size(200, 20));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 20, 300, 380), content->bounds());
  EXPECT_EQ(gfx::Rect(0, 0, 200, 20), top_view->bounds());
}

TEST_F(TopViewLayoutTest, ChangingTopViewOverlay) {
  layout_.reset(new TopViewLayout);

  View* content = new View();
  host_->AddChildView(content);
  layout_->set_content_view(content);

  View* top_view = new ViewWithPreferredSize(gfx::Size(200, 20));
  host_->AddChildView(top_view);
  layout_->set_top_view(top_view);

  host_->SetSize(gfx::Size(200, 150));
  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 20, 200, 130), content->bounds());
  EXPECT_EQ(gfx::Rect(0, 0, 200, 20), top_view->bounds());

  EXPECT_FALSE(layout_->IsUsingOverlay());
  layout_->SetUseOverlay(true);
  EXPECT_TRUE(layout_->IsUsingOverlay());

  layout_->Layout(host_.get());
  EXPECT_EQ(gfx::Rect(0, 0, 200, 150), content->bounds());
  EXPECT_EQ(gfx::Rect(0, 0, 200, 20), top_view->bounds());
}
