// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_DELEGATE_H_

#include <jni.h>
#include <string>

#include "components/web_contents_delegate_android/web_contents_delegate_android.h"

namespace xwalk {

class XWalkWebContentsDelegate
    : public web_contents_delegate_android::WebContentsDelegateAndroid {
 public:
  XWalkWebContentsDelegate(JNIEnv* env, jobject obj);
  virtual ~XWalkWebContentsDelegate();

  void AddNewContents(content::WebContents* source,
                      content::WebContents* new_contents,
                      WindowOpenDisposition disposition,
                      const gfx::Rect& initial_pos,
                      bool user_gesture,
                      bool* was_blocked) override;
  void CloseContents(content::WebContents* source) override;
  void ActivateContents(content::WebContents* contents) override;
  void UpdatePreferredSize(content::WebContents* web_contents,
                                   const gfx::Size& pref_size) override;
  void RunFileChooser(
      content::WebContents* web_contents,
      const content::FileChooserParams& params) override;
  content::JavaScriptDialogManager*
      GetJavaScriptDialogManager() override;

  void RequestMediaAccessPermission(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback) override;

  void RendererUnresponsive(content::WebContents* source) override;
  void RendererResponsive(content::WebContents* source) override;

  void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) override;

  void ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                          bool enter_fullscreen) override;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) const override;

  bool ShouldCreateWebContents(
      content::WebContents* web_contents,
      int route_id,
      WindowContainerType window_container_type,
      const base::string16& frame_name,
      const GURL& target_url,
      const std::string& partition_id,
      content::SessionStorageNamespace* session_storage_namespace) override;

 private:
  scoped_ptr<content::JavaScriptDialogManager> javascript_dialog_manager_;
  DISALLOW_COPY_AND_ASSIGN(XWalkWebContentsDelegate);
};

bool RegisterXWalkWebContentsDelegate(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_DELEGATE_H_
