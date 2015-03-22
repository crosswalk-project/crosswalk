// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/webui/xwalk_web_ui_controller_factory.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "xwalk/runtime/browser/ui/webui/file_picker/file_picker_ui.h"
#include "xwalk/runtime/common/url_constants.h"

namespace xwalk {

content::WebUI::TypeID XWalkWebUIControllerFactory::GetWebUIType(
    content::BrowserContext* browser_context, const GURL& url) const {
  if (url.host() == xwalk::kChromeUIFilePickerHost)
    return const_cast<XWalkWebUIControllerFactory*>(this);
  return content::WebUI::kNoWebUI;
}

bool XWalkWebUIControllerFactory::UseWebUIForURL(
    content::BrowserContext* browser_context, const GURL& url) const {
  return GetWebUIType(browser_context, url) != content::WebUI::kNoWebUI;
}

bool XWalkWebUIControllerFactory::UseWebUIBindingsForURL(
    content::BrowserContext* browser_context, const GURL& url) const {
  return UseWebUIForURL(browser_context, url);
}

content::WebUIController*
XWalkWebUIControllerFactory::CreateWebUIControllerForURL(
    content::WebUI* web_ui, const GURL& url) const {
  if (url.host() == xwalk::kChromeUIFilePickerHost)
    return new ui::FilePickerUI(web_ui);

  return NULL;
}

// static
XWalkWebUIControllerFactory* XWalkWebUIControllerFactory::GetInstance() {
  return Singleton<XWalkWebUIControllerFactory>::get();
}

XWalkWebUIControllerFactory::XWalkWebUIControllerFactory() {
}

XWalkWebUIControllerFactory::~XWalkWebUIControllerFactory() {
}

}  // namespace xwalk
