// Copyright 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_DIALOG_MANAGER_H_
#define XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_DIALOG_MANAGER_H_

#include <memory>
#include <string>

#include "base/memory/singleton.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "url/gurl.h"

namespace xwalk {

class XWalkPermissionDialogManager
    : public content::WebContentsObserver,
      public content::WebContentsUserData<XWalkPermissionDialogManager> {
 public:
  static XWalkPermissionDialogManager* GetPermissionDialogManager(
      content::WebContents* web_contents);
  void RequestPermission(
      ContentSettingsType type,
      const GURL& origin_url,
      const std::string& accept_lang,
      const base::string16& message_text,
      const base::Callback<void(bool)>& callback);

  void CancelPermissionRequest();

 private:
  friend class content::WebContentsUserData<XWalkPermissionDialogManager>;

  explicit XWalkPermissionDialogManager(content::WebContents* web_contents);
  ~XWalkPermissionDialogManager() override;

  void OnPermissionDialogClosed(
      ContentSettingsType type,
      const GURL& origin_url,
      const base::Callback<void(bool)>& callback,
      bool success);

  void WebContentsDestroyed() override;

  content::WebContents* web_contents_;

  DISALLOW_COPY_AND_ASSIGN(XWalkPermissionDialogManager);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_DIALOG_MANAGER_H_
