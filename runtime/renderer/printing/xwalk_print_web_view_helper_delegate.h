// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_PRINTING_XWALK_PRINT_WEB_VIEW_HELPER_DELEGATE_H_
#define XWALK_RUNTIME_RENDERER_PRINTING_XWALK_PRINT_WEB_VIEW_HELPER_DELEGATE_H_

#include "components/printing/renderer/print_web_view_helper.h"

class XWalkPrintWebViewHelperDelegate
    : public printing::PrintWebViewHelper::Delegate {
 public:
  ~XWalkPrintWebViewHelperDelegate() override;

  bool CancelPrerender(content::RenderView* render_view,
                       int routing_id) override;

  blink::WebElement GetPdfElement(blink::WebLocalFrame* frame) override;

  bool IsPrintPreviewEnabled() override;
  bool IsAskPrintSettingsEnabled() override;
  bool IsScriptedPrintEnabled() override;

  bool OverridePrint(blink::WebLocalFrame* frame) override;
};  // class XWalkPrintWebViewHelperDelegate

#endif  // XWALK_RUNTIME_RENDERER_PRINTING_XWALK_PRINT_WEB_VIEW_HELPER_DELEGATE_H_
