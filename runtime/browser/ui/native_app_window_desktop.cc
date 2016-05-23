// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_desktop.h"

#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "grit/xwalk_strings.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/window.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/gfx/canvas.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "xwalk/runtime/browser/devtools/xwalk_devtools_frontend.h"
#include "xwalk/runtime/browser/ui/top_view_layout_views.h"
#include "xwalk/runtime/browser/ui/desktop/download_views.h"
#include "xwalk/runtime/browser/runtime_select_file_policy.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {

struct DownloadSelectFileParams {
  DownloadSelectFileParams(content::DownloadItem* item,
      const content::DownloadTargetCallback& callback)
      : item(item),
        callback(callback) {}
  ~DownloadSelectFileParams() {}

  content::DownloadItem* item;
  content::DownloadTargetCallback callback;
};

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

class NativeAppWindowDesktop::DevToolsWebContentsObserver
    : public content::WebContentsObserver {
 public:
  DevToolsWebContentsObserver(NativeAppWindowDesktop* window,
                              content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
    window_(window) {
  }

  // WebContentsObserver
  void WebContentsDestroyed() override {
    window_->OnDevToolsWebContentsDestroyed();
  }

 private:
  NativeAppWindowDesktop* window_;

  DISALLOW_COPY_AND_ASSIGN(DevToolsWebContentsObserver);
};

// Model for the "Debug" menu
class NativeAppWindowDesktop::ContextMenuModel
  : public ui::SimpleMenuModel,
    public ui::SimpleMenuModel::Delegate {
 public:
  explicit ContextMenuModel(
    NativeAppWindowDesktop* shell, const content::ContextMenuParams& params)
    : ui::SimpleMenuModel(this),
    shell_(shell),
    params_(params) {
    const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
    if (command_line.HasSwitch(switches::kXWalkEnableInspector))
      AddItem(COMMAND_OPEN_DEVTOOLS, base::ASCIIToUTF16("Inspect Element"));
  }

  // ui::SimpleMenuModel::Delegate:
  bool IsCommandIdChecked(int command_id) const override { return false; }
  bool IsCommandIdEnabled(int command_id) const override { return true; }
  bool GetAcceleratorForCommandId(int command_id,
    ui::Accelerator* accelerator) override {
    return false;
  }
  void ExecuteCommand(int command_id, int event_flags) override {
    switch (command_id) {
    case COMMAND_OPEN_DEVTOOLS:
      shell_->ShowDevToolsForElementAt(params_.x, params_.y);
      break;
    }
  }

 private:
  enum CommandID {
    COMMAND_OPEN_DEVTOOLS
  };

  NativeAppWindowDesktop* shell_;
  content::ContextMenuParams params_;

  DISALLOW_COPY_AND_ASSIGN(ContextMenuModel);
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
      contents_view_(nullptr),
      download_bar_view_(nullptr),
      devtools_frontend_(nullptr) {
}

NativeAppWindowDesktop::~NativeAppWindowDesktop() {
}

bool NativeAppWindowDesktop::PlatformHandleContextMenu(
  const content::ContextMenuParams& params) {
  ShowWebViewContextMenu(params);
  return true;
}

void NativeAppWindowDesktop::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this) {
    if (create_params().display_mode == blink::WebDisplayModeMinimalUi)
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

void NativeAppWindowDesktop::FocusContent() {
  web_contents_->GetRenderViewHost()->GetWidget()->Focus();
}

void NativeAppWindowDesktop::ShowWebViewContextMenu(
    const content::ContextMenuParams& params) {
  gfx::Point screen_point(params.x, params.y);

  // Convert from content coordinates to window coordinates.
  // This code copied from chrome_web_contents_view_delegate_views.cc
  aura::Window* web_contents_window =
      web_contents_->GetNativeView();
  aura::Window* root_window = web_contents_window->GetRootWindow();
  aura::client::ScreenPositionClient* screen_position_client =
      aura::client::GetScreenPositionClient(root_window);
  if (screen_position_client) {
    screen_position_client->ConvertPointToScreen(web_contents_window,
        &screen_point);
  }

  context_menu_model_.reset(new ContextMenuModel(this, params));
  context_menu_runner_.reset(new views::MenuRunner(
      context_menu_model_.get(), views::MenuRunner::CONTEXT_MENU));

  if (context_menu_model_->GetItemCount() > 0 &&
      context_menu_runner_->RunMenuAt(web_view_->GetWidget(),
      NULL,
      gfx::Rect(screen_point, gfx::Size()),
      views::MENU_ANCHOR_TOPRIGHT,
      ui::MENU_SOURCE_NONE) ==
      views::MenuRunner::MENU_DELETED) {
    return;
  }
}

void NativeAppWindowDesktop::ShowDevToolsForElementAt(int x, int y) {
  InnerShowDevTools();
  devtools_frontend_->InspectElementAt(x, y);
}

void NativeAppWindowDesktop::InnerShowDevTools() {
  if (!devtools_frontend_) {
    devtools_frontend_ = XWalkDevToolsFrontend::Show(web_contents_);
    devtools_observer_.reset(new DevToolsWebContentsObserver(
      this, devtools_frontend_->frontend_shell()->web_contents_));
  }

  devtools_frontend_->Activate();
  devtools_frontend_->Focus();
}

void NativeAppWindowDesktop::OnDevToolsWebContentsDestroyed() {
  devtools_observer_.reset();
  devtools_frontend_ = nullptr;
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
  int h = height();
  if (toolbar_view_) {
    h -= toolbar_view_->GetPreferredSize().height();
  }

  if (download_bar_view_) {
    h -= download_bar_view_->GetPreferredSize().height();
  }
  web_view_->SetPreferredSize(gfx::Size(width(), h));
}

void NativeAppWindowDesktop::AddDownloadItem(
    content::DownloadItem* download_item,
    const content::DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) {
  select_file_dialog_ =
      ui::SelectFileDialog::Create(this, new RuntimeSelectFilePolicy);
  ui::SelectFileDialog::FileTypeInfo file_type_info;
  file_type_info.include_all_files = true;
  file_type_info.allowed_paths = ui::SelectFileDialog::FileTypeInfo::ANY_PATH;
  DownloadSelectFileParams* params =
      new DownloadSelectFileParams(download_item, callback);
  select_file_dialog_->SelectFile(ui::SelectFileDialog::SELECT_SAVEAS_FILE,
      base::string16(),
      suggested_path,
      &file_type_info,
      0,
      base::FilePath::StringType(),
      GetNativeWindow(),
      params);
}

void NativeAppWindowDesktop::FileSelected(const base::FilePath& path,
    int index,
    void* params) {
  std::unique_ptr<DownloadSelectFileParams> scoped_params(
      static_cast<DownloadSelectFileParams*>(params));
  content::DownloadItem* item = scoped_params->item;
  content::DownloadTargetCallback& callback = scoped_params->callback;

  if (!download_bar_view_) {
    download_bar_view_ = new DownloadBarView(this);
    AddChildView(download_bar_view_);
  }
  download_bar_view_->AddDownloadItem(item, path);

  callback.Run(path, content::DownloadItem::TARGET_DISPOSITION_PROMPT,
               content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS, path);
}

void NativeAppWindowDesktop::FileSelectionCanceled(void* params) {
  std::unique_ptr<DownloadSelectFileParams> scoped_params(
      static_cast<DownloadSelectFileParams*>(params));
  const base::FilePath empty;
  scoped_params->callback.Run(empty,
      content::DownloadItem::TARGET_DISPOSITION_PROMPT,
      content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
      empty);
}

}  // namespace xwalk
