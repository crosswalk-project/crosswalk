// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_SPEECH_SPEECH_RECOGNITION_MANAGER_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_SPEECH_SPEECH_RECOGNITION_MANAGER_DELEGATE_H_

#include <string>

#include "content/public/browser/speech_recognition_event_listener.h"
#include "content/public/browser/speech_recognition_manager_delegate.h"
#include "content/public/browser/speech_recognition_session_config.h"

namespace xwalk {

// This is CrossWalk's implementation of the SpeechRecognitionManagerDelegate
// interface.
class XWalkSpeechRecognitionManagerDelegate
    : public content::SpeechRecognitionManagerDelegate {
 public:
  XWalkSpeechRecognitionManagerDelegate();
  virtual ~XWalkSpeechRecognitionManagerDelegate();

 protected:
  // SpeechRecognitionManagerDelegate methods.
  virtual void GetDiagnosticInformation(bool* can_report_metrics,
                                        std::string* hardware_info) OVERRIDE;
  virtual void CheckRecognitionIsAllowed(
      int session_id,
      base::Callback<void(bool ask_user, bool is_allowed)> callback) OVERRIDE;
  virtual content::SpeechRecognitionEventListener* GetEventListener() OVERRIDE;
  virtual bool FilterProfanities(int render_process_id) OVERRIDE;

 private:
  // Checks for VIEW_TYPE_TAB_CONTENTS host in the UI thread and notifies back
  // the result in the IO thread through |callback|.
  static void CheckRenderViewType(
      base::Callback<void(bool ask_user, bool is_allowed)> callback,
      int render_process_id,
      int render_view_id);

  DISALLOW_COPY_AND_ASSIGN(XWalkSpeechRecognitionManagerDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_SPEECH_SPEECH_RECOGNITION_MANAGER_DELEGATE_H_
