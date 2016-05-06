// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_presentation_service_helper.h"
#include "xwalk/runtime/browser/xwalk_presentation_service_helper_android.h"

namespace xwalk {

DisplayInfoManagerServiceAndroid::DisplayInfoManagerServiceAndroid() {}

DisplayInfoManagerServiceAndroid::~DisplayInfoManagerServiceAndroid() {}

void DisplayInfoManagerServiceAndroid::FindAllAvailableMonitors(
    std::vector<DisplayInfo>* info_list) {
  if (auto presentation_api = XWalkPresentationHost::GetInstance()) {
    auto vec = presentation_api->GetAndroidDisplayInfo();
    for (auto data : vec) {
      DisplayInfo info = {};
      info.name = data.name;
      info.id = std::to_string(data.id);
      info.is_primary = data.is_primary;

      info_list->push_back(info);
    }
  } else {
    LOG(ERROR) << "XWalkPresentationHost instance not found";
  }
}

void DisplayInfoManagerServiceAndroid::DisplayChangeCallback(int) {
  DisplayInfoManager::GetInstance()->UpdateInfoList();
}

void DisplayInfoManagerServiceAndroid::ListenMonitorsUpdate() {
  if (auto presentation_api = XWalkPresentationHost::GetInstance()) {
    presentation_api->SetDisplayChangeCallback(DisplayChangeCallback);
  } else {
    LOG(ERROR) << "Failed to listen to Android Display change event";
  }
}

void DisplayInfoManagerServiceAndroid::StopListenMonitorsUpdate() {
  if (auto presentation_api = XWalkPresentationHost::GetInstance()) {
    presentation_api->SetDisplayChangeCallback(nullptr);
  } else {
  }
}

PresentationSessionAndroid::PresentationSessionAndroid(
    const std::string& presentation_url,
    const std::string& presentation_id,
    const SystemString& display_id)
    : PresentationSession(presentation_url, presentation_id, display_id) {}

PresentationSessionAndroid::~PresentationSessionAndroid() {
  Close();
}

void PresentationSessionAndroid::Create(
    const PresentationSession::CreateParams& params,
    PresentationSession::SessionCallback callback) {
  if (auto presentation_api = XWalkPresentationHost::GetInstance()) {
    int displayId = std::stoi(params.display_info.id);
    presentation_api->ShowPresentation(params.render_process_id,
                                       params.render_frame_id, displayId,
                                       params.presentation_url);

    scoped_refptr<PresentationSession> session(new PresentationSessionAndroid(
        params.presentation_url, params.presentation_id,
        params.display_info.id));
    session->set_render_process_id(params.render_process_id);
    session->set_render_frame_id(params.render_frame_id);
    callback.Run(session, "");
  } else {
    LOG(ERROR) << "XWalkPresentationHost instance not found";
  }
}

void PresentationSessionAndroid::Close() {
  if (auto presentation_api = XWalkPresentationHost::GetInstance()) {
    presentation_api->closePresentation(get_render_process_id(),
                                        get_render_frame_id());
  }
}

PresentationFrameAndroid::PresentationFrameAndroid(
    const RenderFrameHostId& render_frame_host_id)
    : PresentationFrame(render_frame_host_id) {
  if (XWalkPresentationHost* p = XWalkPresentationHost::GetInstance()) {
    p->AddSessionObserver(this);
  }
}

PresentationFrameAndroid::~PresentationFrameAndroid() {
  if (auto presentation_api = XWalkPresentationHost::GetInstance()) {
    presentation_api->RemoveSessionObserver(this);
  }
}

void PresentationFrameAndroid::OnPresentationClosed(int render_process_id,
                                                    int render_frame_id) {
  if (GetRenderFrameHostId().first == render_process_id &&
      GetRenderFrameHostId().second == render_frame_id) {
    if (session())
      session()->NotifyClose();
  }
}

}  // namespace xwalk
