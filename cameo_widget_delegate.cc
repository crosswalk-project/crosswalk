#include "cameo/cameo_widget_delegate.h"

#include "base/utf_string_conversions.h"
#include "ui/views/layout/fill_layout.h"
#include <iostream>

CameoWidgetDelegate::CameoWidgetDelegate() {
  set_background(views::Background::CreateSolidBackground(200, 0, 0));
  SetLayoutManager(new views::FillLayout);
}

string16 CameoWidgetDelegate::GetWindowTitle() const OVERRIDE {
  return ASCIIToUTF16("Cameo");
}

void CameoWidgetDelegate::WindowClosing() OVERRIDE {
  std::cout << "WindowClosing()\n";
}
