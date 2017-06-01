// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.

#include "xwalk/runtime/renderer/printing/xwalk_print_web_view_helper_delegate.h"

#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebElement.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"


XWalkPrintWebViewHelperDelegate::~XWalkPrintWebViewHelperDelegate(){
}

bool XWalkPrintWebViewHelperDelegate::CancelPrerender(
    content::RenderView* render_view, int routing_id) {
  return false;
}

blink::WebElement XWalkPrintWebViewHelperDelegate::GetPdfElement(
        blink::WebLocalFrame* frame) {
  return blink::WebElement();
}

bool XWalkPrintWebViewHelperDelegate::IsPrintPreviewEnabled() {
  return false;
}

bool XWalkPrintWebViewHelperDelegate::IsAskPrintSettingsEnabled() {
  return true;
}

bool XWalkPrintWebViewHelperDelegate::IsScriptedPrintEnabled() {
  return true;
}

bool XWalkPrintWebViewHelperDelegate::OverridePrint(
    blink::WebLocalFrame* frame) {
  return false;
}
