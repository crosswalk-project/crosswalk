// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_presentation_service_delegate_android.h"

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
#include "xwalk/runtime/browser/android/xwalk_presentation_host.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(xwalk::
  XWalkPresentationServiceDelegateAndroid);

namespace xwalk {

using application::Application;
using application::ApplicationService;

using content::PresentationConnectionStateChangedCallback;
using content::PresentationScreenAvailabilityListener;
using content::PresentationSessionMessage;
using content::RenderFrameHost;

using DelegateObserver = content::PresentationServiceDelegate::Observer;
using PresentationSessionErrorCallback =
    content::PresentationSessionErrorCallback;
using PresentationSessionStartedCallback =
    content::PresentationSessionStartedCallback;
using RenderFrameHostId =
    XWalkPresentationServiceDelegateAndroid::RenderFrameHostId;
using SessionInfo = content::PresentationSessionInfo;
using SystemString = std::basic_string<char>;

namespace {

bool IsValidPresentationUrl(const std::string& url) {
  GURL gurl(url);
  return gurl.is_valid();
}

}  // namespace

struct DisplayInfo {
  gfx::Rect bounds;
  bool is_primary;
  bool in_use;
  SystemString name;
  SystemString id;
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

  ~DisplayInfoManager();

  static DisplayInfoManager* GetInstance();

  const std::vector<DisplayInfo>& info_list() const { return info_list_; }

  const DisplayInfo* FindAvailable() const;
  bool IsStillAvailable(const SystemString& display_id) const;

  bool MarkAsUsed(const SystemString& id, bool in_use);

  void AddObserver(Observer* observer) {
    observers_.AddObserver(observer);
  }

  void RemoveObserver(Observer* observer) {
    observers_.RemoveObserver(observer);
  }

 private:
  static void DisplayChangeCallback(int display_id);

 private:
  DisplayInfoManager();
  friend struct base::DefaultSingletonTraits<DisplayInfoManager>;

  void platformFindAllAvailableMonitors();
  void UpdateInfoList();
  void ListenMonitorsUpdate();
  void StopListenMonitorsUpdate();
  void NotifyInfoChanged();

  std::vector<DisplayInfo> info_list_;
  base::Closure callback_;
  base::ObserverList<Observer> observers_;
};

DisplayInfoManager* DisplayInfoManager::GetInstance() {
  return base::Singleton<DisplayInfoManager>::get();
}

DisplayInfoManager::DisplayInfoManager() {
  UpdateInfoList();
  ListenMonitorsUpdate();
}

DisplayInfoManager::~DisplayInfoManager() {
  StopListenMonitorsUpdate();
}

const DisplayInfo* DisplayInfoManager::FindAvailable() const {
  for (const DisplayInfo& info : info_list_) {
    if (!info.is_primary && !info.in_use)
      return &info;
  }
  return nullptr;
}

bool DisplayInfoManager::IsStillAvailable(
    const SystemString& display_id) const {
  for (const DisplayInfo& info : info_list_) {
    if (!info.is_primary && info.id == display_id)
      return true;
  }
  return false;
}

bool DisplayInfoManager::MarkAsUsed(
    const SystemString& id, bool in_use) {
  for (DisplayInfo& info : info_list_) {
    if (info.id == id) {
      if (in_use != info.in_use) {
        info.in_use = in_use;
        NotifyInfoChanged();
      }
      return true;
    }
  }
  return false;
}

void DisplayInfoManager::platformFindAllAvailableMonitors() {
  if ( auto presentation_api = XWalkPresentationHost::GetInstance() ) {
    auto vec = presentation_api->GetAndroidDisplayInfo();
    for ( auto data : vec ) {
      DisplayInfo info = {};
      info.name = data.name;
      info.id = std::to_string(data.id);
      info.is_primary = data.is_primary;

      this->info_list_.push_back(info);
    }
  } else {
    LOG(ERROR) << "XWalkPresentationHost instance not found";
  }
}

void DisplayInfoManager::UpdateInfoList() {
  std::vector<SystemString> ids_in_use;
  for (DisplayInfo& info : this->info_list_) {
    if (info.in_use)
      ids_in_use.push_back(info.id);
  }

  this->info_list_.clear();
  this->platformFindAllAvailableMonitors();

  for ( auto& data : info_list_ ) {
    auto found = std::find(ids_in_use.begin(), ids_in_use.end(), data.id);
    if ( found != ids_in_use.end() ) {
      // Mark the id as "in_use" after re-scan monitors list
      data.in_use = true;
    }
  }

  // Broadcast the change notification
  NotifyInfoChanged();
}

void DisplayInfoManager::DisplayChangeCallback(int /*display_id*/) {
  DisplayInfoManager::GetInstance()->UpdateInfoList();
}

void DisplayInfoManager::ListenMonitorsUpdate() {
  if ( auto presentation_api = XWalkPresentationHost::GetInstance() ) {
    presentation_api->SetDisplayChangeCallback(DisplayChangeCallback);
  } else {
    LOG(ERROR) << "Failed to listen to Android Display change event";
  }
}

void DisplayInfoManager::StopListenMonitorsUpdate() {
  if ( auto presentation_api = XWalkPresentationHost::GetInstance() ) {
    presentation_api->SetDisplayChangeCallback(nullptr);
  } else {
  }
}

void DisplayInfoManager::NotifyInfoChanged() {
  FOR_EACH_OBSERVER(Observer, observers_,
                OnDisplayInfoChanged(info_list_));
}

class PresentationSession :
    public Runtime::Observer,
    public base::RefCounted<PresentationSession> {
 public:
  class Observer {
   public:
    virtual void OnPresentationSessionClosed(const SessionInfo& session_info) {
    }
   protected:
    virtual ~Observer() {}
  };

  int render_process_id_;
  int render_frame_id_;

  struct CreateParams {
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

  static void Create(const CreateParams& params, SessionCallback callback);

  void Close();

  void AddObserver(Observer* observer) {
    observers_.AddObserver(observer);
  }

  void RemoveObserver(Observer* observer) {
    observers_.RemoveObserver(observer);
  }

  const SessionInfo& session_info() const {
    return session_info_;
  }

  SystemString display_id() const { return display_id_; }

#if defined(OS_ANDROID)
  void OnAndoridPresentationClosed();
#endif

 protected:
  ~PresentationSession() override;
  friend base::RefCounted<PresentationSession>;

 private:
  PresentationSession(
      const std::string& presentation_url,
      const std::string& presentation_id,
      const SystemString& display_id);
  void OnNewRuntimeAdded(Runtime* new_runtime) override;
  void OnRuntimeClosed(Runtime* runtime) override;
  void OnApplicationExitRequested(Runtime* runtime) override;
  void NotifyClose();

  SessionInfo session_info_;
  SystemString display_id_;
  ScopedVector<Runtime> runtimes_;
  base::ObserverList<Observer> observers_;

  base::WeakPtrFactory<PresentationSession> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(PresentationSession);
};

PresentationSession::PresentationSession(
    const std::string& presentation_url,
    const std::string& presentation_id,
    const SystemString& display_id)
  : session_info_(presentation_url, presentation_id),
    display_id_(display_id),
    weak_factory_(this) {
}

PresentationSession::~PresentationSession() {
  Close();
}

void PresentationSession::Create(
    const PresentationSession::CreateParams& params,
    PresentationSession::SessionCallback callback) {

  if ( auto presentation_api = XWalkPresentationHost::GetInstance() ) {
    int displayId = std::stoi(params.display_info.id);
    presentation_api->ShowPresentation(params.render_process_id,
      params.render_frame_id, displayId, params.presentation_url);

    scoped_refptr<PresentationSession> session(
        new PresentationSession(
            params.presentation_url,
            params.presentation_id,
            params.display_info.id));
    session->render_process_id_ = params.render_process_id;
    session->render_frame_id_ = params.render_frame_id;
    callback.Run(session, "");

  } else {
    LOG(ERROR) << "XWalkPresentationHost instance not found";
  }
}

void PresentationSession::Close() {
  if ( auto presentation_api = XWalkPresentationHost::GetInstance() ) {
    presentation_api->closePresentation(this->render_process_id_,
      this->render_frame_id_);
  }
}

void PresentationSession::OnNewRuntimeAdded(Runtime* runtime) {
  runtimes_.push_back(runtime);
  runtime->set_observer(this);
  // TODO(Mikhail): handle show popups in presentation context.
}

void PresentationSession::OnRuntimeClosed(Runtime* runtime) {
  auto found = std::find(runtimes_.begin(), runtimes_.end(), runtime);
  CHECK(found != runtimes_.end());
  runtimes_.erase(found);

  if (runtimes_.empty()) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&PresentationSession::NotifyClose,
        weak_factory_.GetWeakPtr()));
  }
}

void PresentationSession::OnApplicationExitRequested(Runtime* runtime) {
}

void PresentationSession::NotifyClose() {
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnPresentationSessionClosed(session_info_));
}

#if defined(OS_ANDROID)
void PresentationSession::OnAndoridPresentationClosed() {
  this->NotifyClose();
}
#endif

// Used by PresentationServiceDelegateImpl to manage
// listeners and default presentation info in a render frame.
class PresentationFrame : public PresentationSession::Observer,
#if defined(OS_ANDROID)
                          public XWalkPresentationHost::SessionObserver,
#endif
                          public DisplayInfoManager::Observer {
 public:
  explicit PresentationFrame(const RenderFrameHostId& render_frame_host_id);
  ~PresentationFrame() override;

  // Mirror corresponding APIs in PresentationServiceDelegateImpl.
  bool SetScreenAvailabilityListener(
      PresentationScreenAvailabilityListener* listener);
  bool RemoveScreenAvailabilityListener(
      PresentationScreenAvailabilityListener* listener);
  void ListenForConnectionStateChange(
      const content::PresentationSessionInfo& connection,
      const content::PresentationConnectionStateChangedCallback&
          state_changed_cb);
  void Reset();

  void OnPresentationSessionStarted(
      scoped_refptr<PresentationSession> session);

  void set_delegate_observer(DelegateObserver* observer) {
    delegate_observer_ = observer;
  }

  void set_default_presentation_url(const std::string& url) {
    default_presentation_url_ = url;
  }

  PresentationSession* session() { return session_.get(); }

#if defined(OS_ANDROID)
  virtual void OnPresentationClosed(int render_process_id,
    int render_frame_id) override;
#endif

 private:
  // PresentationSession::Observer overrides.
  void OnPresentationSessionClosed(
      const SessionInfo& session_info) override;

  // DisplayInfoManager::Observer overrides.
  void OnDisplayInfoChanged(
      const std::vector<DisplayInfo>& info_list) override;

  std::string default_presentation_url_;
  DelegateObserver* delegate_observer_;
  scoped_refptr<PresentationSession> session_;
  PresentationConnectionStateChangedCallback state_changed_cb_;
  PresentationScreenAvailabilityListener* screen_listener_;
  RenderFrameHostId render_frame_host_id_;
};

PresentationFrame::PresentationFrame(
  const RenderFrameHostId& render_frame_host_id)
  : screen_listener_(nullptr),
    render_frame_host_id_(render_frame_host_id) {
  DisplayInfoManager::GetInstance()->AddObserver(this);
#if defined(OS_ANDROID)
  if ( XWalkPresentationHost* p = XWalkPresentationHost::GetInstance() ) {
    p->AddSessionObserver(this);
  }
#endif
}

PresentationFrame::~PresentationFrame() {
  if (delegate_observer_)
    delegate_observer_->OnDelegateDestroyed();
  if (session_)
    session_->RemoveObserver(this);
  DisplayInfoManager::GetInstance()->RemoveObserver(this);
#if defined(OS_ANDROID)
  if ( auto presentation_api = XWalkPresentationHost::GetInstance() ) {
    presentation_api->RemoveSessionObserver(this);
  }
#endif
}

void PresentationFrame::OnPresentationSessionStarted(
    scoped_refptr<PresentationSession> session) {
  session_ = session;
  session_->AddObserver(this);
  DisplayInfoManager::GetInstance()->MarkAsUsed(session_->display_id(), true);

  if (!state_changed_cb_.is_null()) {
    state_changed_cb_.Run(content::PRESENTATION_CONNECTION_STATE_CONNECTED);
  }
}

void PresentationFrame::OnPresentationSessionClosed(
    const SessionInfo& session_info) {
  if (!state_changed_cb_.is_null()) {
    state_changed_cb_.Run(content::PRESENTATION_CONNECTION_STATE_CLOSED);
  }

  DisplayInfoManager::GetInstance()->MarkAsUsed(session_->display_id(), false);
  session_ = nullptr;
}

#if defined(OS_ANDROID)
void PresentationFrame::OnPresentationClosed(int render_process_id,
    int render_frame_id) {
  if ( this->render_frame_host_id_.first == render_process_id
    && this->render_frame_host_id_.second == render_frame_id ) {
    this->session_->OnAndoridPresentationClosed();
  }
}
#endif

void PresentationFrame::OnDisplayInfoChanged(
    const std::vector<DisplayInfo>& info_list) {
  if (screen_listener_) {
    if ( auto presentation_session = this->session() ) {
      // When system display is changed (e.g. rotated),
      //  check if it the display owned by me is still available
      auto my_display_id = presentation_session->display_id();
      bool still_available = DisplayInfoManager::GetInstance()->
        IsStillAvailable(my_display_id);
      screen_listener_->OnScreenAvailabilityChanged(still_available);
    } else {
      screen_listener_->OnScreenAvailabilityChanged(
        DisplayInfoManager::GetInstance()->FindAvailable() != nullptr);
    }
  }

  if (!session_)
    return;
  bool display_found = false;
  for (const DisplayInfo& info : info_list) {
    if (session_->display_id() == info.id) {
      display_found = true;
      break;
    }
  }
  if (!display_found) {
    // The display has been disconnected.
    session_->Close();
  }
}

bool PresentationFrame::SetScreenAvailabilityListener(
    PresentationScreenAvailabilityListener* listener) {
  if (screen_listener_ == listener)
    return false;

  screen_listener_ = listener;
  if (screen_listener_) {
    screen_listener_->OnScreenAvailabilityChanged(
        DisplayInfoManager::GetInstance()->FindAvailable() != nullptr);
  }
  return true;
}

bool PresentationFrame::RemoveScreenAvailabilityListener(
    PresentationScreenAvailabilityListener* listener) {
  if (screen_listener_ == listener) {
    screen_listener_ = nullptr;
    return true;
  }
  return false;
}

void PresentationFrame::Reset() {
  default_presentation_url_.clear();
  session_ = nullptr;
  screen_listener_ = nullptr;
  state_changed_cb_.Reset();
}

void PresentationFrame::ListenForConnectionStateChange(
    const content::PresentationSessionInfo& connection,
    const content::PresentationConnectionStateChangedCallback&
        state_changed_cb) {
  CHECK(state_changed_cb_.is_null());
  state_changed_cb_ = state_changed_cb;
}

content::PresentationServiceDelegate* XWalkPresentationServiceDelegateAndroid::
    GetOrCreateForWebContents(content::WebContents* web_contents) {
  DCHECK(web_contents);

  // CreateForWebContents does nothing if the delegate instance already exists.
  XWalkPresentationServiceDelegateAndroid::CreateForWebContents(web_contents);
  return XWalkPresentationServiceDelegateAndroid::FromWebContents(web_contents);
}

XWalkPresentationServiceDelegateAndroid::
  XWalkPresentationServiceDelegateAndroid(
    content::WebContents* web_contents)
    : web_contents_(web_contents) {
}

XWalkPresentationServiceDelegateAndroid::
  ~XWalkPresentationServiceDelegateAndroid() {
}

void XWalkPresentationServiceDelegateAndroid::AddObserver(
    int render_process_id,
    int render_frame_id,
    Observer* observer) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  presentation_frame->set_delegate_observer(observer);
}

void XWalkPresentationServiceDelegateAndroid::RemoveObserver(
    int render_process_id,
    int render_frame_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  if (presentation_frame) {
    presentation_frame->set_delegate_observer(nullptr);
    presentation_frames_.erase(id);
  }
}

bool XWalkPresentationServiceDelegateAndroid::AddScreenAvailabilityListener(
    int render_process_id,
    int render_frame_id,
    PresentationScreenAvailabilityListener* listener) {
  DCHECK(listener);
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  return presentation_frame->SetScreenAvailabilityListener(listener);
}

void XWalkPresentationServiceDelegateAndroid::RemoveScreenAvailabilityListener(
    int render_process_id,
    int render_frame_id,
    PresentationScreenAvailabilityListener* listener) {
  DCHECK(listener);
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);
  presentation_frame->RemoveScreenAvailabilityListener(listener);
}

void XWalkPresentationServiceDelegateAndroid::Reset(
    int render_process_id,
    int render_frame_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);
  presentation_frame->Reset();
}

void XWalkPresentationServiceDelegateAndroid::SetDefaultPresentationUrl(
    int render_process_id,
    int render_frame_id,
    const std::string& default_presentation_url,
    const PresentationSessionStartedCallback& callback) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  presentation_frame->set_default_presentation_url(default_presentation_url);
}

void XWalkPresentationServiceDelegateAndroid::OnSessionStarted(
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
  error_cb.Run(content::PresentationError(content::PRESENTATION_ERROR_UNKNOWN,
                                          error));
}

void XWalkPresentationServiceDelegateAndroid::StartSession(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_url,
    const PresentationSessionStartedCallback& success_cb,
    const PresentationSessionErrorCallback& error_cb) {
  if (presentation_url.empty() || !IsValidPresentationUrl(presentation_url)) {
    error_cb.Run(content::PresentationError(content::PRESENTATION_ERROR_UNKNOWN,
                                            "Invalid presentation arguments."));
    return;
  }

  const DisplayInfo* available_monitor =
      DisplayInfoManager::GetInstance()->FindAvailable();
  if (!available_monitor) {
    error_cb.Run(content::PresentationError(
        content::PRESENTATION_ERROR_NO_AVAILABLE_SCREENS,
        "No available monitors"));
    return;
  }

  RenderFrameHostId render_frame_host_id(render_process_id, render_frame_id);
  const std::string& presentation_id = base::GenerateGUID();

  PresentationSession::CreateParams params = {};
  params.display_info = *available_monitor;
  params.presentation_id = presentation_id;
  params.presentation_url = presentation_url;
  params.web_contents = web_contents_;
  params.render_process_id = render_process_id;
  params.render_frame_id = render_frame_id;
  params.application = nullptr;

  auto callback = base::Bind(
      &XWalkPresentationServiceDelegateAndroid::OnSessionStarted,
      AsWeakPtr(), render_frame_host_id, success_cb, error_cb);
  PresentationSession::Create(params, callback);
}

void XWalkPresentationServiceDelegateAndroid::JoinSession(
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
      "There is no session with id: " + presentation_id + ", and URL: "
      + presentation_url));
}

void XWalkPresentationServiceDelegateAndroid::CloseConnection(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_id) {
  NOTIMPLEMENTED();
}

void XWalkPresentationServiceDelegateAndroid::Terminate(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);

  if (auto session = presentation_frame->session())
    session->Close();
}

void XWalkPresentationServiceDelegateAndroid::ListenForConnectionStateChange(
    int render_process_id,
    int render_frame_id,
    const content::PresentationSessionInfo& connection,
    const content::PresentationConnectionStateChangedCallback&
        state_changed_cb) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  if (presentation_frame) {
    presentation_frame->ListenForConnectionStateChange(connection,
                                                       state_changed_cb);
  }
}

PresentationFrame* XWalkPresentationServiceDelegateAndroid::
    GetOrAddPresentationFrame(const RenderFrameHostId& render_frame_host_id) {
  if (!presentation_frames_.contains(render_frame_host_id)) {
    presentation_frames_.add(
        render_frame_host_id,
        scoped_ptr<PresentationFrame>(
            new PresentationFrame(render_frame_host_id)));
  }
  return presentation_frames_.get(render_frame_host_id);
}

}  // namespace xwalk
