// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_CREATOR_H_
#define CAMEO_SRC_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_CREATOR_H_

namespace content {
class WebContents;
class WebContentsViewDelegate;
}

namespace cameo {

content::WebContentsViewDelegate* CreateShellWebContentsViewDelegate(
    content::WebContents* web_contents);

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_SHELL_WEB_CONTENTS_VIEW_DELEGATE_CREATOR_H_
