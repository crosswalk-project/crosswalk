// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_presentation_service_delegate.h"

#include <string>
#include <vector>

#include "base/guid.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/singleton.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/presentation_screen_availability_listener.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/browser/xwalk_presentation_service_helper.h"

namespace xwalk {

XWalkPresentationServiceDelegate::XWalkPresentationServiceDelegate(
    content::WebContents* web_contents)
    : web_contents_(web_contents) {}

XWalkPresentationServiceDelegate::~XWalkPresentationServiceDelegate() {}

void XWalkPresentationServiceDelegate::AddObserver(int render_process_id,
                                                   int render_frame_id,
                                                   Observer* observer) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  presentation_frame->set_delegate_observer(observer);
}

void XWalkPresentationServiceDelegate::RemoveObserver(int render_process_id,
                                                      int render_frame_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  if (presentation_frame) {
    presentation_frame->set_delegate_observer(nullptr);
    presentation_frames_.erase(id);
  }
}

bool XWalkPresentationServiceDelegate::AddScreenAvailabilityListener(
    int render_process_id,
    int render_frame_id,
    PresentationScreenAvailabilityListener* listener) {
  DCHECK(listener);
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  return presentation_frame->SetScreenAvailabilityListener(listener);
}

void XWalkPresentationServiceDelegate::RemoveScreenAvailabilityListener(
    int render_process_id,
    int render_frame_id,
    PresentationScreenAvailabilityListener* listener) {
  DCHECK(listener);
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);
  presentation_frame->RemoveScreenAvailabilityListener(listener);
}

void XWalkPresentationServiceDelegate::Reset(int render_process_id,
                                             int render_frame_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);
  presentation_frame->Reset();
}

void XWalkPresentationServiceDelegate::SetDefaultPresentationUrl(
    int render_process_id,
    int render_frame_id,
    const std::string& default_presentation_url,
    const PresentationSessionStartedCallback& callback) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  presentation_frame->set_default_presentation_url(default_presentation_url);
}

void XWalkPresentationServiceDelegate::OnSessionStarted(
    const RenderFrameHostId& id,
    const PresentationSessionStartedCallback& success_cb,
    const PresentationSessionErrorCallback& error_cb,
    scoped_refptr<PresentationSession> session,
    const std::string& error) {
  auto presentation_frame = presentation_frames_.get(id);
  if (presentation_frame && session) {
    presentation_frame->OnPresentationSessionStarted(session);
    success_cb.Run(session->session_info());
    return;
  }
  error_cb.Run(
      content::PresentationError(content::PRESENTATION_ERROR_UNKNOWN, error));
}

void XWalkPresentationServiceDelegate::JoinSession(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_url,
    const std::string& presentation_id,
    const PresentationSessionStartedCallback& success_cb,
    const PresentationSessionErrorCallback& error_cb) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);

  for (auto& frame : presentation_frames_) {
    if (auto session = frame.second->session()) {
      if (session->session_info().presentation_id == presentation_id &&
          session->session_info().presentation_url == presentation_url) {
        presentation_frame->OnPresentationSessionStarted(session);
        SessionInfo info(presentation_url, presentation_id);
        success_cb.Run(info);
        return;
      }
    }
  }

  error_cb.Run(content::PresentationError(
      content::PRESENTATION_ERROR_NO_PRESENTATION_FOUND,
      "There is no session with id: " + presentation_id + ", and URL: " +
          presentation_url));
}

void XWalkPresentationServiceDelegate::Terminate(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_id) {
  CloseConnection(render_process_id, render_frame_id, presentation_id);
}

void XWalkPresentationServiceDelegate::CloseConnection(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);

  if (auto session = presentation_frame->session()) {
    session->Close();
  }
}

void XWalkPresentationServiceDelegate::ListenForConnectionStateChange(
    int render_process_id,
    int render_frame_id,
    const content::PresentationSessionInfo& connection,
    const PresentationConnectionStateChangedCallback& state_changed_cb) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  PresentationFrame* presentation_frame = GetOrAddPresentationFrame(id);
  presentation_frame->ListenForSessionStateChange(state_changed_cb);
}

PresentationFrame* XWalkPresentationServiceDelegate::GetOrAddPresentationFrame(
    const RenderFrameHostId& render_frame_host_id) {
  if (!presentation_frames_.contains(render_frame_host_id)) {
    presentation_frames_.add(render_frame_host_id,
                             PresentationFrame::Create(render_frame_host_id));
  }
  return presentation_frames_.get(render_frame_host_id);
}

}  // namespace xwalk
