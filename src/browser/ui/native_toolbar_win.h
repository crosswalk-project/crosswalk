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

#ifndef CAMEO_SRC_BROWSER_UI_NATIVE_TOOLBAR_WIN_H_
#define CAMEO_SRC_BROWSER_UI_NATIVE_TOOLBAR_WIN_H_

#include <string>

#include "base/basictypes.h"
#include "cameo/src/browser/ui/native_app_window.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/widget/widget_delegate.h"

namespace views {
class ImageButton;
class Textfield;
}

namespace cameo {

class NativeToolbarWin : public views::WidgetDelegateView,
                         public views::TextfieldController,
                         public views::ButtonListener {
 public:
  explicit NativeToolbarWin(Shell* shell);
  ~NativeToolbarWin();

  void SetButtonEnabled(NativeAppWindow::ButtonType button, bool enabled);
  void SetUrlEntry(const std::string& url);
  void SetIsLoading(bool loading);

 protected:
  // Overridden from WidgetDelegateView:
  virtual views::View* GetContentsView() OVERRIDE;

  // Overridden from View:
  virtual void Layout() OVERRIDE;
  virtual void ViewHierarchyChanged(bool is_add,
                                    views::View* parent,
                                    views::View* child) OVERRIDE;

  // Overridden from TextfieldController:
  virtual void ContentsChanged(views::Textfield* sender,
                               const string16& new_contents) OVERRIDE;
  virtual bool HandleKeyEvent(views::Textfield* sender,
                              const ui::KeyEvent& key_event) OVERRIDE;

  // Overridden from ButtonListener:
  virtual void ButtonPressed(views::Button* sender,
                             const ui::Event& event) OVERRIDE;

 private:
  void InitToolbar();

  Shell* shell_;

  views::ImageButton* back_button_;
  views::ImageButton* forward_button_;
  views::ImageButton* stop_or_refresh_button_;
  views::Textfield* url_entry_;
  views::ImageButton* devtools_button_;

  DISALLOW_COPY_AND_ASSIGN(NativeToolbarWin);
};

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_UI_NATIVE_TOOLBAR_WIN_H_
