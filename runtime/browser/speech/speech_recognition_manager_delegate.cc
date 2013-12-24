// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/speech/speech_recognition_manager_delegate.h"

#include <string>

#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/speech_recognition_manager.h"

using content::BrowserThread;
using content::SpeechRecognitionManager;

namespace xwalk {

XWalkSpeechRecognitionManagerDelegate
::XWalkSpeechRecognitionManagerDelegate() {
}

XWalkSpeechRecognitionManagerDelegate
::~XWalkSpeechRecognitionManagerDelegate() {
}

void XWalkSpeechRecognitionManagerDelegate::GetDiagnosticInformation(
    bool* can_report_metrics,
    std::string* hardware_info) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  *can_report_metrics = true;
  string16 device_model =
      SpeechRecognitionManager::GetInstance()->GetAudioInputDeviceModel();
  *hardware_info = UTF16ToUTF8(device_model);
}

void XWalkSpeechRecognitionManagerDelegate::CheckRecognitionIsAllowed(
    int session_id,
    base::Callback<void(bool ask_user, bool is_allowed)> callback) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

  const content::SpeechRecognitionSessionContext& context =
      SpeechRecognitionManager::GetInstance()->GetSessionContext(session_id);

  // Make sure that initiators (extensions/web pages) properly set the
  // |render_process_id| field, which is needed later to retrieve the profile.
  DCHECK_NE(context.render_process_id, 0);

  int render_process_id = context.render_process_id;
  int render_view_id = context.render_view_id;
  if (context.embedder_render_process_id) {
    // If this is a request originated from a guest, we need to re-route the
    // permission check through the embedder (app).
    render_process_id = context.embedder_render_process_id;
    render_view_id = context.embedder_render_view_id;
  }

  // Check that the render view type is appropriate, and whether or not we
  // need to request permission from the user.
  BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
                          base::Bind(&CheckRenderViewType,
                                     callback,
                                     render_process_id,
                                     render_view_id,
                                     !context.requested_by_page_element));
}

content::SpeechRecognitionEventListener*
XWalkSpeechRecognitionManagerDelegate::GetEventListener() {
  return NULL;
}

bool XWalkSpeechRecognitionManagerDelegate::FilterProfanities(
    int render_process_id) {
  content::RenderProcessHost* rph =
      content::RenderProcessHost::FromID(render_process_id);
  return rph == NULL;
}

// static.
void XWalkSpeechRecognitionManagerDelegate::CheckRenderViewType(
    base::Callback<void(bool ask_user, bool is_allowed)> callback,
    int render_process_id,
    int render_view_id,
    bool js_api) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const content::RenderViewHost* render_view_host =
      content::RenderViewHost::FromID(render_process_id, render_view_id);

  bool allowed = (render_view_host != NULL);
  bool check_permission = false;

  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
                          base::Bind(callback, check_permission, allowed));
}

}  // namespace xwalk
