// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_WIN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_WIN_H_

#include <string>
#include <utility>

#include "base/containers/scoped_ptr_hash_map.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/presentation_service_delegate.h"
#include "content/public/browser/web_contents_user_data.h"

namespace content {
class WebContents;
}  // namespace content

namespace xwalk {

class PresentationFrame;
class PresentationSession;
class XWalkBrowserContext;

class XWalkPresentationServiceDelegateWin
    : public content::WebContentsUserData<XWalkPresentationServiceDelegateWin>,
      public content::PresentationServiceDelegate,
      public base::SupportsWeakPtr<XWalkPresentationServiceDelegateWin> {
 public:
  using RenderFrameHostId = std::pair<int, int>;

  static content::PresentationServiceDelegate* GetOrCreateForWebContents(
      content::WebContents* web_contents);

  ~XWalkPresentationServiceDelegateWin() override;

 private:
  explicit XWalkPresentationServiceDelegateWin(
      content::WebContents* web_contents);
  friend class
      content::WebContentsUserData<XWalkPresentationServiceDelegateWin>;

  void AddObserver(int render_process_id,
      int render_frame_id,
      Observer* observer) override;

  void RemoveObserver(int render_process_id, int render_frame_id) override;

  bool AddScreenAvailabilityListener(
      int render_process_id,
      int render_frame_id,
      content::PresentationScreenAvailabilityListener* listener) override;

  void RemoveScreenAvailabilityListener(
      int render_process_id,
      int render_frame_id,
      content::PresentationScreenAvailabilityListener* listener) override;

  void Reset(
      int render_process_id,
      int render_frame_id) override;

  void SetDefaultPresentationUrl(
      int render_process_id,
      int render_frame_id,
      const std::string& default_presentation_url,
      const content::PresentationSessionStartedCallback& callback) override;

  void StartSession(
      int render_process_id,
      int render_frame_id,
      const std::string& presentation_url,
      const content::PresentationSessionStartedCallback& success_cb,
      const content::PresentationSessionErrorCallback& error_cb) override;

  void JoinSession(
      int render_process_id,
      int render_frame_id,
      const std::string& presentation_url,
      const std::string& presentation_id,
      const content::PresentationSessionStartedCallback& success_cb,
      const content::PresentationSessionErrorCallback& error_cb) override;

  void CloseConnection(
      int render_process_id,
      int render_frame_id,
      const std::string& presentation_id) override;

  void Terminate(int render_process_id,
                 int render_frame_id,
                 const std::string& presentation_id) override;

  void ListenForSessionMessages(
      int render_process_id,
      int render_frame_id,
      const content::PresentationSessionInfo& session,
      const content::PresentationSessionMessageCallback& message_cb)
      override {}

  void SendMessage(
      int render_process_id,
      int render_frame_id,
      const content::PresentationSessionInfo& session,
      scoped_ptr<content::PresentationSessionMessage> message_request,
      const SendMessageCallback& send_message_cb) override {}

  void ListenForConnectionStateChange(
      int render_process_id,
      int render_frame_id,
      const content::PresentationSessionInfo& connection,
      const content::PresentationConnectionStateChangedCallback&
          state_changed_cb) override;

 private:
  PresentationFrame* GetOrAddPresentationFrame(
      const RenderFrameHostId& render_frame_host_id);
  void OnSessionStarted(
      const RenderFrameHostId& id,
      const content::PresentationSessionStartedCallback& success_cb,
      const content::PresentationSessionErrorCallback& error_cb,
      scoped_refptr<PresentationSession> session,
      const std::string& error);

  content::WebContents* web_contents_;
  base::ScopedPtrHashMap<RenderFrameHostId, scoped_ptr<PresentationFrame>>
      presentation_frames_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_DELEGATE_WIN_H_
