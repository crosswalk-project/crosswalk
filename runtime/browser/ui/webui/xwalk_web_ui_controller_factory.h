// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_WEBUI_XWALK_WEB_UI_CONTROLLER_FACTORY_H_
#define XWALK_RUNTIME_BROWSER_UI_WEBUI_XWALK_WEB_UI_CONTROLLER_FACTORY_H_

#include "base/basictypes.h"
#include "base/memory/singleton.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_controller_factory.h"

namespace xwalk {

class XWalkWebUIControllerFactory : public content::WebUIControllerFactory {
 public:
  content::WebUI::TypeID GetWebUIType(content::BrowserContext* browser_context,
                             const GURL& url) const override;
  bool UseWebUIForURL(content::BrowserContext* browser_context,
                      const GURL& url) const override;
  bool UseWebUIBindingsForURL(content::BrowserContext* browser_context,
                              const GURL& url) const override;
  content::WebUIController* CreateWebUIControllerForURL(content::WebUI* web_ui,
                                               const GURL& url) const override;

  static XWalkWebUIControllerFactory* GetInstance();

 protected:
  XWalkWebUIControllerFactory();
  ~XWalkWebUIControllerFactory() override;

 private:
  friend struct DefaultSingletonTraits<XWalkWebUIControllerFactory>;

  DISALLOW_COPY_AND_ASSIGN(XWalkWebUIControllerFactory);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_WEBUI_XWALK_WEB_UI_CONTROLLER_FACTORY_H_
