// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge_base.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

using content::BrowserThread;
using content::WebContents;

namespace xwalk {

namespace {

const void* kXWalkContentsClientBridgeBase = &kXWalkContentsClientBridgeBase;

// This class is invented so that the UserData registry that we inject the
// XWalkContentsClientBridgeBase object does not own and destroy it.
class UserData : public base::SupportsUserData::Data {
 public:
  static XWalkContentsClientBridgeBase* GetContents(
      content::WebContents* web_contents) {
    if (!web_contents)
      return NULL;
    UserData* data = reinterpret_cast<UserData*>(
        web_contents->GetUserData(kXWalkContentsClientBridgeBase));
    return data ? data->contents_ : NULL;
  }

  explicit UserData(XWalkContentsClientBridgeBase* ptr) : contents_(ptr) {}
 private:
  XWalkContentsClientBridgeBase* contents_;

  DISALLOW_COPY_AND_ASSIGN(UserData);
};

}  // namespace

// static
void XWalkContentsClientBridgeBase::Associate(
    WebContents* web_contents,
    XWalkContentsClientBridgeBase* handler) {
  web_contents->SetUserData(kXWalkContentsClientBridgeBase,
                            new UserData(handler));
}

// static
XWalkContentsClientBridgeBase* XWalkContentsClientBridgeBase::FromWebContents(
    WebContents* web_contents) {
  return UserData::GetContents(web_contents);
}

// static
XWalkContentsClientBridgeBase* XWalkContentsClientBridgeBase::FromRenderViewID(
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

// static
XWalkContentsClientBridgeBase* XWalkContentsClientBridgeBase::FromRenderFrameID(
    int render_process_id,
    int render_frame_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  content::RenderFrameHost* rfh =
      content::RenderFrameHost::FromID(render_process_id, render_frame_id);
  if (!rfh) return NULL;
  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(rfh);
  return UserData::GetContents(web_contents);
}

XWalkContentsClientBridgeBase::~XWalkContentsClientBridgeBase() {
}

}  // namespace xwalk
