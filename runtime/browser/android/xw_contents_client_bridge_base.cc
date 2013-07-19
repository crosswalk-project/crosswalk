// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xw_contents_client_bridge_base.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

using content::BrowserThread;
using content::WebContents;

namespace xwalk {

namespace {

const void* kXwContentsClientBridgeBase = &kXwContentsClientBridgeBase;

// This class is invented so that the UserData registry that we inject the
// XwContentsClientBridgeBase object does not own and destroy it.
class UserData : public base::SupportsUserData::Data {
 public:
  static XwContentsClientBridgeBase* GetContents(
      content::WebContents* web_contents) {
    if (!web_contents)
      return NULL;
    UserData* data = reinterpret_cast<UserData*>(
        web_contents->GetUserData(kXwContentsClientBridgeBase));
    return data ? data->contents_ : NULL;
  }

  explicit UserData(XwContentsClientBridgeBase* ptr) : contents_(ptr) {}
 private:
  XwContentsClientBridgeBase* contents_;

  DISALLOW_COPY_AND_ASSIGN(UserData);
};

} // namespace

// static
void XwContentsClientBridgeBase::Associate(
    WebContents* web_contents,
    XwContentsClientBridgeBase* handler) {
  web_contents->SetUserData(kXwContentsClientBridgeBase,
                            new UserData(handler));
}

// static
XwContentsClientBridgeBase* XwContentsClientBridgeBase::FromWebContents(
    WebContents* web_contents) {
  return UserData::GetContents(web_contents);
}

// static
XwContentsClientBridgeBase* XwContentsClientBridgeBase::FromID(
    int render_process_id,
    int render_view_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const content::RenderViewHost* rvh =
      content::RenderViewHost::FromID(render_process_id, render_view_id);
  if (!rvh) return NULL;
  content::WebContents* web_contents =
      content::WebContents::FromRenderViewHost(rvh);
  return UserData::GetContents(web_contents);
}

XwContentsClientBridgeBase::~XwContentsClientBridgeBase() {
}

}  // namespace xwalk
