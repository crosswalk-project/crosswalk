// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_DELEGATE_H_

#include <jni.h>

#include "components/web_contents_delegate_android/web_contents_delegate_android.h"

namespace xwalk {

class XWalkWebContentsDelegate
    : public web_contents_delegate_android::WebContentsDelegateAndroid {
 public:
  XWalkWebContentsDelegate(JNIEnv* env, jobject obj);
  virtual ~XWalkWebContentsDelegate();

  virtual void AddNewContents(content::WebContents* source,
                              content::WebContents* new_contents,
                              WindowOpenDisposition disposition,
                              const gfx::Rect& initial_pos,
                              bool user_gesture,
                              bool* was_blocked) OVERRIDE;
  virtual void CloseContents(content::WebContents* source) OVERRIDE;
  virtual void ActivateContents(content::WebContents* contents) OVERRIDE;
  virtual void UpdatePreferredSize(content::WebContents* web_contents,
                                   const gfx::Size& pref_size) OVERRIDE;
  virtual void RunFileChooser(
      content::WebContents* web_contents,
      const content::FileChooserParams& params) OVERRIDE;
  virtual content::JavaScriptDialogManager*
      GetJavaScriptDialogManager() OVERRIDE;

  virtual void RequestMediaAccessPermission(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback) OVERRIDE;

  virtual void RendererUnresponsive(content::WebContents* source) OVERRIDE;
  virtual void RendererResponsive(content::WebContents* source) OVERRIDE;

 private:
  scoped_ptr<content::JavaScriptDialogManager> javascript_dialog_manager_;
  DISALLOW_COPY_AND_ASSIGN(XWalkWebContentsDelegate);
};

bool RegisterXWalkWebContentsDelegate(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_DELEGATE_H_
