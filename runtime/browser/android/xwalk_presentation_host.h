// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_PRESENTATION_HOST_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_PRESENTATION_HOST_H_

#include <vector>
#include <string>

#include "base/android/jni_weak_ref.h"
#include "base/android/scoped_java_ref.h"
#include "base/observer_list.h"

namespace xwalk {

class XWalkPresentationHost {
 public:
  struct AndroidDisplay {
    bool is_primary;
    bool is_presentation;
    std::string name;
    int id;
  };

  class SessionObserver {
   public:
    virtual void OnPresentationClosed(int render_process_id,
      int render_frame_id) = 0;
  };

 public:
  // Note: It's hard to mark this ctor private and at the
  //       same time declare friend func to the below func:
  //         static jlong Init(JNIEnv* env, jobject obj);
  //       The above function just has to be 'static'
  //
  //       So just keep it public at this point.
  XWalkPresentationHost(JNIEnv* env, jobject obj);
  ~XWalkPresentationHost();

  typedef void (*DisplayChangeCallbackType)(int);
  typedef void (*PresentationClosedCallbackType)(int, int);

  void SetDisplayChangeCallback(DisplayChangeCallbackType func) {
    display_change_callback_ = func;
  }

  void AddSessionObserver(SessionObserver* observer) {
    observers_.AddObserver(observer);
  }

  void RemoveSessionObserver(SessionObserver* observer) {
    observers_.RemoveObserver(observer);
  }

  void Destroy(JNIEnv* env, jobject obj);
  void SetupJavaPeer(JNIEnv* env, jobject obj);

  void OnDisplayAdded(JNIEnv* env, jobject obj, int display_id);
  void OnDisplayChanged(JNIEnv* env, jobject obj, int display_id);
  void OnDisplayRemoved(JNIEnv* env, jobject obj, int display_id);
  void OnPresentationClosed(JNIEnv* env, jobject obj,
    const int render_process_id, const int render_frame_id);

  std::vector<XWalkPresentationHost::AndroidDisplay>
    GetAndroidDisplayInfo() const;
  bool ShowPresentation(const int render_process_id,
    const int render_frame_id,
    const int display_id, const std::string& url);
  void closePresentation(const int render_process_id,
    const int render_frame_id);

 private:
  JavaObjectWeakGlobalRef java_ref_;

 private:
  DisplayChangeCallbackType display_change_callback_;
  base::ObserverList<SessionObserver> observers_;

 private:
  static XWalkPresentationHost* gInstance;

 public:
  static XWalkPresentationHost* GetInstance();  // Not a normal singleton

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkPresentationHost);
};

bool RegisterXWalkPresentationHost(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_PRESENTATION_HOST_H_
