// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell.h"

#include "base/command_line.h"
#include "base/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "ui/aura/env.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window.h"
#include "ui/base/accessibility/accessibility_types.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/events/event.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/screen.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/view.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

#if defined(OS_CHROMEOS)
#include "chromeos/dbus/dbus_thread_manager.h"
#include "cameo/src/minimal_ash.h"
#include "ui/aura/test/test_screen.h"
#endif

// ViewDelegate implementation for aura content shell
class ShellViewsDelegateAura : public views::DesktopTestViewsDelegate {
 public:
  ShellViewsDelegateAura() : use_transparent_windows_(false) {
  }

  virtual ~ShellViewsDelegateAura() {
    ViewsDelegate::views_delegate = NULL;
  }

  void SetUseTransparentWindows(bool transparent) {
    use_transparent_windows_ = transparent;
  }

  // Overridden from views::TestViewsDelegate:
  virtual bool UseTransparentWindows() const OVERRIDE {
    return use_transparent_windows_;
  }

 private:
  bool use_transparent_windows_;

  DISALLOW_COPY_AND_ASSIGN(ShellViewsDelegateAura);
};

// TODO(beng): This stuff should NOT be in the views namespace!
namespace views {

// Maintain the UI controls and web view for content shell
class ShellWindowDelegateView : public WidgetDelegateView,
                                public TextfieldController,
                                public ButtonListener {
 public:
  enum UIControl {
    BACK_BUTTON,
    FORWARD_BUTTON,
    STOP_BUTTON
  };

  ShellWindowDelegateView(content::Shell* shell)
    : shell_(shell),
      toolbar_view_(new View),
      contents_view_(new View) {
  }
  virtual ~ShellWindowDelegateView() {}

  // Update the state of UI controls
  void SetAddressBarURL(const GURL& url) {
    url_entry_->SetText(ASCIIToUTF16(url.spec()));
  }
  void SetWebContents(content::WebContents* web_contents) {
    contents_view_->SetLayoutManager(new FillLayout());
    web_view_ = new WebView(web_contents->GetBrowserContext());
    web_view_->SetWebContents(web_contents);
    web_contents->GetView()->Focus();
    contents_view_->AddChildView(web_view_);
    Layout();
  }
  void SetWindowTitle(const string16& title) { title_ = title; }
  void EnableUIControl(UIControl control, bool is_enabled) {
    if (control == BACK_BUTTON) {
      back_button_->SetState(is_enabled ? CustomButton::STATE_NORMAL
          : CustomButton::STATE_DISABLED);
    } else if (control == FORWARD_BUTTON) {
      forward_button_->SetState(is_enabled ? CustomButton::STATE_NORMAL
          : CustomButton::STATE_DISABLED);
    } else if (control == STOP_BUTTON) {
      stop_button_->SetState(is_enabled ? CustomButton::STATE_NORMAL
          : CustomButton::STATE_DISABLED);
    }
  }

 private:
  // Initialize the UI control contained in shell window
  void InitShellWindow() {
    set_background(Background::CreateStandardPanelBackground());

    GridLayout* layout = new GridLayout(this);
    SetLayoutManager(layout);

    ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, 2);
    column_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1,
                          GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, 2);

    layout->AddPaddingRow(0, 2);

    // Add toolbar buttons and URL text field
    {
      layout->StartRow(0, 0);
      GridLayout* toolbar_layout = new GridLayout(toolbar_view_);
      toolbar_view_->SetLayoutManager(toolbar_layout);

      ColumnSet* toolbar_column_set =
          toolbar_layout->AddColumnSet(0);
      // Back button
      back_button_ = new LabelButton(this, ASCIIToUTF16("Back"));
      back_button_->SetStyle(views::Button::STYLE_NATIVE_TEXTBUTTON);
      gfx::Size back_button_size = back_button_->GetPreferredSize();
      toolbar_column_set->AddColumn(GridLayout::CENTER,
                                    GridLayout::CENTER, 0,
                                    GridLayout::FIXED,
                                    back_button_size.width(),
                                    back_button_size.width() / 2);
      // Forward button
      forward_button_ = new LabelButton(this, ASCIIToUTF16("Forward"));
      forward_button_->SetStyle(views::Button::STYLE_NATIVE_TEXTBUTTON);
      gfx::Size forward_button_size = forward_button_->GetPreferredSize();
      toolbar_column_set->AddColumn(GridLayout::CENTER,
                                    GridLayout::CENTER, 0,
                                    GridLayout::FIXED,
                                    forward_button_size.width(),
                                    forward_button_size.width() / 2);
      // Refresh button
      refresh_button_ = new LabelButton(this, ASCIIToUTF16("Refresh"));
      refresh_button_->SetStyle(views::Button::STYLE_NATIVE_TEXTBUTTON);
      gfx::Size refresh_button_size = refresh_button_->GetPreferredSize();
      toolbar_column_set->AddColumn(GridLayout::CENTER,
                                    GridLayout::CENTER, 0,
                                    GridLayout::FIXED,
                                    refresh_button_size.width(),
                                    refresh_button_size.width() / 2);
      // Stop button
      stop_button_ = new LabelButton(this, ASCIIToUTF16("Stop"));
      stop_button_->SetStyle(views::Button::STYLE_NATIVE_TEXTBUTTON);
      gfx::Size stop_button_size = stop_button_->GetPreferredSize();
      toolbar_column_set->AddColumn(GridLayout::CENTER,
                                    GridLayout::CENTER, 0,
                                    GridLayout::FIXED,
                                    stop_button_size.width(),
                                    stop_button_size.width() / 2);
      toolbar_column_set->AddPaddingColumn(0, 2);
      // URL entry
      url_entry_ = new Textfield();
      url_entry_->SetController(this);
      toolbar_column_set->AddColumn(GridLayout::FILL,
                                    GridLayout::FILL, 1,
                                    GridLayout::USE_PREF, 0, 0);

      // Fill up the first row
      toolbar_layout->StartRow(0, 0);
      toolbar_layout->AddView(back_button_);
      toolbar_layout->AddView(forward_button_);
      toolbar_layout->AddView(refresh_button_);
      toolbar_layout->AddView(stop_button_);
      toolbar_layout->AddView(url_entry_);

      layout->AddView(toolbar_view_);
    }

    layout->AddPaddingRow(0, 5);

    // Add web contents view as the second row
    {
      layout->StartRow(1, 0);
      layout->AddView(contents_view_);
    }

    layout->AddPaddingRow(0, 5);
  }
  // Overridden from TextfieldController
  virtual void ContentsChanged(Textfield* sender,
                               const string16& new_contents) OVERRIDE {
  }
  virtual bool HandleKeyEvent(Textfield* sender,
                              const ui::KeyEvent& key_event) OVERRIDE {
   if (sender == url_entry_ && key_event.key_code() == ui::VKEY_RETURN) {
     std::string text = UTF16ToUTF8(url_entry_->text());
     GURL url(text);
     if (!url.has_scheme()) {
       url = GURL(std::string("http://") + std::string(text));
       url_entry_->SetText(ASCIIToUTF16(url.spec()));
     }
     shell_->LoadURL(url);
     return true;
   }
   return false;
  }

  // Overridden from ButtonListener
  virtual void ButtonPressed(Button* sender, const ui::Event& event) OVERRIDE {
    if (sender == back_button_)
      shell_->GoBackOrForward(-1);
    else if (sender == forward_button_)
      shell_->GoBackOrForward(1);
    else if (sender == refresh_button_)
      shell_->Reload();
    else if (sender == stop_button_)
      shell_->Stop();
  }

  // Overridden from WidgetDelegateView
  virtual bool CanResize() const OVERRIDE { return true; }
  virtual bool CanMaximize() const OVERRIDE { return true; }
  virtual string16 GetWindowTitle() const OVERRIDE {
    return title_;
  }
  virtual void WindowClosing() OVERRIDE {
    if (shell_) {
      delete shell_;
      shell_ = NULL;
    }
  }
  virtual View* GetContentsView() OVERRIDE { return this; }

  // Overridden from View
  virtual void ViewHierarchyChanged(bool is_add,
                                    View* parent,
                                    View* child) OVERRIDE {
    if (is_add && child == this) {
      InitShellWindow();
    }
  }

 private:
  // Hold a reference of Shell for deleting it when the window is closing
  content::Shell* shell_;

  // Window title
  string16 title_;

  // Toolbar view contains forward/backward/reload button and URL entry
  View* toolbar_view_;
  LabelButton* back_button_;
  LabelButton* forward_button_;
  LabelButton* refresh_button_;
  LabelButton* stop_button_;
  Textfield* url_entry_;

  // Contents view contains the web contents view
  View* contents_view_;
  WebView* web_view_;

  DISALLOW_COPY_AND_ASSIGN(ShellWindowDelegateView);
};

}  // namespace views

using views::ShellWindowDelegateView;

namespace content {

#if defined(OS_CHROMEOS)
MinimalAsh* Shell::minimal_ash_ = NULL;
#endif
views::ViewsDelegate* Shell::views_delegate_ = NULL;

// static
void Shell::PlatformInitialize(const gfx::Size& default_window_size) {
#if defined(OS_CHROMEOS)
  chromeos::DBusThreadManager::Initialize();
  gfx::Screen::SetScreenInstance(
      gfx::SCREEN_TYPE_NATIVE, aura::TestScreen::Create());
  minimal_ash_ = new content::MinimalAsh(default_window_size);
#else
  gfx::Screen::SetScreenInstance(
      gfx::SCREEN_TYPE_NATIVE, views::CreateDesktopScreen());
#endif
  views_delegate_ = new ShellViewsDelegateAura();
}

void Shell::PlatformExit() {
#if defined(OS_CHROMEOS)
  if (minimal_ash_)
    delete minimal_ash_;
#endif
  if (views_delegate_)
    delete views_delegate_;
#if defined(OS_CHROMEOS)
  chromeos::DBusThreadManager::Shutdown();
#endif
  aura::Env::DeleteInstance();
}

void Shell::PlatformCleanUp() {
}

void Shell::PlatformEnableUIControl(UIControl control, bool is_enabled) {
  ShellWindowDelegateView* delegate_view =
    static_cast<ShellWindowDelegateView*>(window_widget_->widget_delegate());
  if (control == BACK_BUTTON) {
    delegate_view->EnableUIControl(ShellWindowDelegateView::BACK_BUTTON,
        is_enabled);
  } else if (control == FORWARD_BUTTON) {
    delegate_view->EnableUIControl(ShellWindowDelegateView::FORWARD_BUTTON,
        is_enabled);
  } else if (control == STOP_BUTTON) {
    delegate_view->EnableUIControl(ShellWindowDelegateView::STOP_BUTTON,
        is_enabled);
  }
}

void Shell::PlatformSetAddressBarURL(const GURL& url) {
  ShellWindowDelegateView* delegate_view =
    static_cast<ShellWindowDelegateView*>(window_widget_->widget_delegate());
  delegate_view->SetAddressBarURL(url);
}

void Shell::PlatformSetIsLoading(bool loading) {
}

void Shell::PlatformCreateWindow(int width, int height) {
#if defined(OS_CHROMEOS)
  window_widget_ =
      views::Widget::CreateWindowWithContextAndBounds(
          new ShellWindowDelegateView(this),
          minimal_ash_->GetDefaultParent(NULL, NULL, gfx::Rect()),
          gfx::Rect(0, 0, width, height));
#else
  window_widget_ =
      views::Widget::CreateWindowWithBounds(new ShellWindowDelegateView(this),
               gfx::Rect(0, 0, width, height));
#endif

  window_ = window_widget_->GetNativeWindow();
  // Call ShowRootWindow on RootWindow created by MinimalAsh without
  // which XWindow owned by RootWindow doesn't get mapped.
  window_->GetRootWindow()->ShowRootWindow();
  window_widget_->Show();
}

void Shell::PlatformSetContents() {
  ShellWindowDelegateView* delegate_view =
    static_cast<ShellWindowDelegateView*>(window_widget_->widget_delegate());
  delegate_view->SetWebContents(web_contents_.get());
}

void Shell::PlatformResizeSubViews() {
}

void Shell::Close() {
  window_widget_->Close();
}

void Shell::PlatformSetTitle(const string16& title) {
  ShellWindowDelegateView* delegate_view =
    static_cast<ShellWindowDelegateView*>(window_widget_->widget_delegate());
  delegate_view->SetWindowTitle(title);
  window_widget_->UpdateWindowTitle();
}

}  // namespace content
