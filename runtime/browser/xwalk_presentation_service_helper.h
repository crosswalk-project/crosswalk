// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_H_

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

#include "xwalk/runtime/browser/xwalk_presentation_service_delegate.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_presentation_host.h"
#elif defined(OS_WIN)
#include <Windows.h>
#endif

namespace xwalk {

using application::Application;
using application::ApplicationService;

using content::PresentationScreenAvailabilityListener;
using content::PresentationSessionMessage;
using content::RenderFrameHost;
using content::PresentationConnectionStateChangedCallback;

using DelegateObserver = content::PresentationServiceDelegate::Observer;
using PresentationSessionErrorCallback =
    content::PresentationSessionErrorCallback;
using PresentationSessionStartedCallback =
    content::PresentationSessionStartedCallback;
using RenderFrameHostId = XWalkPresentationServiceDelegate::RenderFrameHostId;
using SessionInfo = content::PresentationSessionInfo;

#if defined(OS_ANDROID)
using SystemString = std::basic_string<char>;
#elif defined(OS_WIN)
using SystemString = std::basic_string<TCHAR>;
#else
using SystemString = std::basic_string<char>;
#endif

inline bool IsValidPresentationUrl(const std::string& url) {
  GURL gurl(url);
  return gurl.is_valid();
}

inline Application* GetApplication(content::WebContents* contents) {
  auto app_service =
      XWalkRunner::GetInstance()->app_system()->application_service();
  int rph_id = contents->GetRenderProcessHost()->GetID();
  return app_service->GetApplicationByRenderHostID(rph_id);
}

struct DisplayInfo {
  DisplayInfo();

  gfx::Rect bounds;
  bool is_primary;
  bool in_use;
  SystemString name;
  SystemString id;
};

// Platform-dependent service interface for DisplayInfoManager class
class DisplayInfoManagerService {
 public:
  static std::unique_ptr<DisplayInfoManagerService> Create();
  virtual ~DisplayInfoManagerService() {}
  virtual void FindAllAvailableMonitors(
      std::vector<DisplayInfo>* info_list) = 0;
  virtual void ListenMonitorsUpdate() = 0;
  virtual void StopListenMonitorsUpdate() = 0;
};

// This class provides up-to-date info about the available
// display monitors (including wireless).
class DisplayInfoManager {
 public:
  class Observer {
   public:
    virtual void OnDisplayInfoChanged(
        const std::vector<DisplayInfo>& info_list) {}

   protected:
    virtual ~Observer() {}
  };

 public:
  static DisplayInfoManager* GetInstance();
  ~DisplayInfoManager();

  const std::vector<DisplayInfo>& info_list() const { return info_list_; }

  const DisplayInfo* FindAvailable() const;
  bool IsStillAvailable(const SystemString& display_id) const;

  bool MarkAsUsed(const SystemString& id, bool in_use);

  void AddObserver(Observer* observer) { observers_.AddObserver(observer); }

  void RemoveObserver(Observer* observer) {
    observers_.RemoveObserver(observer);
  }

  void NotifyInfoChanged();

  void UpdateInfoList();

 public:
  void FindAllAvailableMonitors();
  void ListenMonitorsUpdate();
  void StopListenMonitorsUpdate();

 private:
  friend struct base::DefaultSingletonTraits<DisplayInfoManager>;
  DisplayInfoManager();
  std::vector<DisplayInfo> info_list_;

 private:
  base::ObserverList<Observer> observers_;
  std::unique_ptr<DisplayInfoManagerService> service_;
};

class PresentationSession : public base::RefCounted<PresentationSession> {
 public:
  class Observer {
   public:
    virtual void OnPresentationSessionClosed(const SessionInfo& session_info) {}

   protected:
    virtual ~Observer() {}
  };

  struct CreateParams {
    CreateParams();

    content::WebContents* web_contents;
    Application* application;
    std::string presentation_id;
    std::string presentation_url;
    int render_process_id;
    int render_frame_id;
    DisplayInfo display_info;
  };

  using SessionCallback =
      base::Callback<void(scoped_refptr<PresentationSession>,
                          const std::string& error)>;

  virtual void Close() = 0;

  void AddObserver(Observer* observer) { observers_.AddObserver(observer); }

  void RemoveObserver(Observer* observer) {
    observers_.RemoveObserver(observer);
  }

  const SessionInfo& session_info() const { return session_info_; }

  SystemString display_id() const { return display_id_; }

  void NotifyClose();

  int get_render_process_id() const { return render_process_id_; }
  int get_render_frame_id() const { return render_frame_id_; }

  void set_render_process_id(int id) { render_process_id_ = id; }
  void set_render_frame_id(int id) { render_frame_id_ = id; }

 protected:
  PresentationSession(const std::string& presentation_url,
                      const std::string& presentation_id,
                      const SystemString& display_id);

  virtual ~PresentationSession();

  friend class base::RefCounted<PresentationSession>;

  int render_process_id_;
  int render_frame_id_;
  SessionInfo session_info_;
  SystemString display_id_;
  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<PresentationSession> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(PresentationSession);
};

// Used by PresentationServiceDelegateImpl to manage
// listeners and default presentation info in a render frame.
class PresentationFrame : public PresentationSession::Observer,
                          public DisplayInfoManager::Observer {
 public:
  static std::unique_ptr<PresentationFrame> Create(
      const RenderFrameHostId& render_frame_host_id);
  ~PresentationFrame() override;

  // Mirror corresponding APIs in PresentationServiceDelegateImpl.
  bool SetScreenAvailabilityListener(
      PresentationScreenAvailabilityListener* listener);
  bool RemoveScreenAvailabilityListener(
      PresentationScreenAvailabilityListener* listener);
  void ListenForSessionStateChange(
      const content::PresentationConnectionStateChangedCallback&
          state_changed_cb);
  void Reset();

  void OnPresentationSessionStarted(scoped_refptr<PresentationSession> session);

  void set_delegate_observer(DelegateObserver* observer) {
    delegate_observer_ = observer;
  }

  void set_default_presentation_url(const std::string& url) {
    default_presentation_url_ = url;
  }

  PresentationSession* session() { return session_.get(); }

 protected:
  explicit PresentationFrame(const RenderFrameHostId& render_frame_host_id);

 private:
  // PresentationSession::Observer overrides.
  void OnPresentationSessionClosed(const SessionInfo& session_info) override;

  // DisplayInfoManager::Observer overrides.
  void OnDisplayInfoChanged(const std::vector<DisplayInfo>& info_list) override;

  std::string default_presentation_url_;
  DelegateObserver* delegate_observer_;
  scoped_refptr<PresentationSession> session_;
  PresentationConnectionStateChangedCallback state_changed_cb_;
  PresentationScreenAvailabilityListener* screen_listener_;
  RenderFrameHostId render_frame_host_id_;

 protected:
  const RenderFrameHostId& GetRenderFrameHostId() const {
    return render_frame_host_id_;
  }
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_H_
