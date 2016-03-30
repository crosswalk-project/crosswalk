// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_presentation_service_delegate_win.h"

#include <string>
#include <vector>

#include <Windows.h>

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

DEFINE_WEB_CONTENTS_USER_DATA_KEY(xwalk::XWalkPresentationServiceDelegateWin);

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
    XWalkPresentationServiceDelegateWin::RenderFrameHostId;
using SessionInfo = content::PresentationSessionInfo;
using SystemString = std::basic_string<TCHAR>;

namespace {

bool IsValidPresentationUrl(const std::string& url) {
  GURL gurl(url);
  return gurl.is_valid();
}

Application* GetApplication(content::WebContents* contents) {
  auto app_service =
      XWalkRunner::GetInstance()->app_system()->application_service();
  int rph_id = contents->GetRenderProcessHost()->GetID();
  return app_service->GetApplicationByRenderHostID(rph_id);
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

  bool MarkAsUsed(const SystemString& id, bool in_use);

  void AddObserver(Observer* observer) {
    observers_.AddObserver(observer);
  }

  void RemoveObserver(Observer* observer) {
    observers_.RemoveObserver(observer);
  }

 private:
  DisplayInfoManager();
  friend struct base::DefaultSingletonTraits<DisplayInfoManager>;

  static BOOL CALLBACK MonitorEnumCallback(
      HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM lParam) {
    MONITORINFOEX info_platform;
    info_platform.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &info_platform);

    DISPLAY_DEVICE display_device = {};
    display_device.cb = sizeof(DISPLAY_DEVICE);
    EnumDisplayDevices(info_platform.szDevice, 0, &display_device, 0);

    DisplayInfo info = {};
    info.bounds = gfx::Rect(*lprcMonitor);
    info.is_primary = info_platform.dwFlags & MONITORINFOF_PRIMARY;
    info.name = display_device.DeviceName;
    info.id = display_device.DeviceID;

    DisplayInfoManager* self = reinterpret_cast<DisplayInfoManager*>(lParam);
    self->info_list_.push_back(info);

    return TRUE;
  }

  static LRESULT CALLBACK WndProcCallback(
      HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_DISPLAYCHANGE) {
      DisplayInfoManager::GetInstance()->UpdateInfoList();
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  void UpdateInfoList();
  void ListenMonitorsUpdate();
  void NotifyInfoChanged();

  std::vector<DisplayInfo> info_list_;
  base::Closure callback_;
  HWND hwnd_;

  base::ObserverList<Observer> observers_;
};

DisplayInfoManager* DisplayInfoManager::GetInstance() {
  return base::Singleton<DisplayInfoManager>::get();
}

DisplayInfoManager::DisplayInfoManager()
  : hwnd_(HWND()) {
  UpdateInfoList();
  ListenMonitorsUpdate();
}

DisplayInfoManager::~DisplayInfoManager() {
  if (hwnd_)
    CloseWindow(hwnd_);
}

const DisplayInfo* DisplayInfoManager::FindAvailable() const {
  for (const DisplayInfo& info : info_list_) {
    if (!info.is_primary && !info.in_use)
      return &info;
  }
  return nullptr;
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

void DisplayInfoManager::UpdateInfoList() {
  std::vector<SystemString> ids_in_use;
  for (DisplayInfo& info : info_list_) {
    if (info.in_use)
      ids_in_use.push_back(info.id);
  }

  info_list_.clear();
  EnumDisplayMonitors(
      0, 0, MonitorEnumCallback, reinterpret_cast<LPARAM>(this));
  NotifyInfoChanged();
}

void DisplayInfoManager::ListenMonitorsUpdate()  {
  const auto class_name = L"_LISTEN_DISPLAYCHANGE";
  WNDCLASSEX wx = {};
  wx.cbSize = sizeof(WNDCLASSEX);
  wx.lpfnWndProc = WndProcCallback;
  wx.hInstance = GetModuleHandle(NULL);
  wx.lpszClassName = class_name;
  if (!RegisterClassEx(&wx)) {
    LOG(ERROR) << "Failed to register a window class for"
               << "listening WM_DISPLAYCHANGE";
    return;
  }

  hwnd_ = CreateWindowEx(
      0, class_name, NULL, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
      HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);
  if (!hwnd_)
    LOG(ERROR) << "Failed to register a window for listening WM_DISPLAYCHANGE";
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

  struct CreateParams {
    content::WebContents* web_contents;
    Application* application;
    std::string presentation_id;
    std::string presentation_url;
    DisplayInfo display_info;
  };

  using SessionCallback =
    base::Callback<void(scoped_refptr<PresentationSession>,
        const std::string& error)>;

  ~PresentationSession() override;

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
  scoped_refptr<PresentationSession> session(
      new PresentationSession(
          params.presentation_url,
          params.presentation_id,
          params.display_info.id));
  XWalkBrowserContext* context =
      XWalkBrowserContext::FromWebContents(params.web_contents);
  DCHECK(context);
  GURL url(params.presentation_url);
  auto site = content::SiteInstance::CreateForURL(context, url);
  Runtime* runtime = Runtime::Create(context, site);
  auto rph = runtime->GetRenderProcessHost();
  if (auto security_policy = params.application->security_policy())
    security_policy->EnforceForRenderer(rph);
  runtime->set_observer(session.get());
  session->runtimes_.push_back(runtime);

  runtime->LoadURL(url);

  NativeAppWindow::CreateParams win_params;
  win_params.bounds = params.display_info.bounds;
  // TODO(Mikhail): provide a special UI delegate for presentation windows.
  auto ui_delegate = RuntimeUIDelegate::Create(runtime, win_params);
  runtime->set_ui_delegate(ui_delegate);
  runtime->Show();
  ui_delegate->SetFullscreen(true);
  callback.Run(session, "");
}

void PresentationSession::Close() {
  std::vector<Runtime*> to_be_closed(runtimes_.get());
  for (Runtime* runtime : to_be_closed)
    runtime->Close();
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

// Used by PresentationServiceDelegateImpl to manage
// listeners and default presentation info in a render frame.
class PresentationFrame : public PresentationSession::Observer,
                          public DisplayInfoManager::Observer {
 public:
  PresentationFrame();
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
};

PresentationFrame::PresentationFrame()
  : screen_listener_(nullptr) {
  DisplayInfoManager::GetInstance()->AddObserver(this);
}

PresentationFrame::~PresentationFrame() {
  if (delegate_observer_)
    delegate_observer_->OnDelegateDestroyed();
  if (session_)
    session_->RemoveObserver(this);
  DisplayInfoManager::GetInstance()->RemoveObserver(this);
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

void PresentationFrame::OnDisplayInfoChanged(
    const std::vector<DisplayInfo>& info_list) {
  if (screen_listener_) {
    screen_listener_->OnScreenAvailabilityChanged(
      DisplayInfoManager::GetInstance()->FindAvailable() != nullptr);
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

content::PresentationServiceDelegate* XWalkPresentationServiceDelegateWin::
    GetOrCreateForWebContents(content::WebContents* web_contents) {
  DCHECK(web_contents);
  Application* app = GetApplication(web_contents);
  if (!app) {
    LOG(WARNING) << "Presentation API is only accessible for applications";
    return nullptr;
  }
  // CreateForWebContents does nothing if the delegate instance already exists.
  XWalkPresentationServiceDelegateWin::CreateForWebContents(web_contents);
  return XWalkPresentationServiceDelegateWin::FromWebContents(web_contents);
}

XWalkPresentationServiceDelegateWin::XWalkPresentationServiceDelegateWin(
    content::WebContents* web_contents)
    : web_contents_(web_contents) {
}

XWalkPresentationServiceDelegateWin::~XWalkPresentationServiceDelegateWin() {
}

void XWalkPresentationServiceDelegateWin::AddObserver(
    int render_process_id,
    int render_frame_id,
    Observer* observer) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  presentation_frame->set_delegate_observer(observer);
}

void XWalkPresentationServiceDelegateWin::RemoveObserver(
    int render_process_id,
    int render_frame_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  if (presentation_frame) {
    presentation_frame->set_delegate_observer(nullptr);
    presentation_frames_.erase(id);
  }
}

bool XWalkPresentationServiceDelegateWin::AddScreenAvailabilityListener(
    int render_process_id,
    int render_frame_id,
    PresentationScreenAvailabilityListener* listener) {
  DCHECK(listener);
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  return presentation_frame->SetScreenAvailabilityListener(listener);
}

void XWalkPresentationServiceDelegateWin::RemoveScreenAvailabilityListener(
    int render_process_id,
    int render_frame_id,
    PresentationScreenAvailabilityListener* listener) {
  DCHECK(listener);
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);
  presentation_frame->RemoveScreenAvailabilityListener(listener);
}

void XWalkPresentationServiceDelegateWin::Reset(
    int render_process_id,
    int render_frame_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);
  presentation_frame->Reset();
}

void XWalkPresentationServiceDelegateWin::SetDefaultPresentationUrl(
    int render_process_id,
    int render_frame_id,
    const std::string& default_presentation_url,
    const PresentationSessionStartedCallback& callback) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = GetOrAddPresentationFrame(id);
  presentation_frame->set_default_presentation_url(default_presentation_url);
}

void XWalkPresentationServiceDelegateWin::OnSessionStarted(
    const RenderFrameHostId& id,
    const content::PresentationSessionStartedCallback& success_cb,
    const content::PresentationSessionErrorCallback& error_cb,
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

void XWalkPresentationServiceDelegateWin::StartSession(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_url,
    const content::PresentationSessionStartedCallback& success_cb,
    const content::PresentationSessionErrorCallback& error_cb) {
  if (presentation_url.empty() || !IsValidPresentationUrl(presentation_url)) {
    error_cb.Run(content::PresentationError(content::PRESENTATION_ERROR_UNKNOWN,
                                            "Invalid presentation arguments."));
    return;
  }
  Application* app = GetApplication(web_contents_);
  CHECK(app);

  if (!app->CanRequestURL(GURL(presentation_url))) {
    error_cb.Run(content::PresentationError(content::PRESENTATION_ERROR_UNKNOWN,
                                            "Failed CSP check."));
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
  params.application = app;

  auto callback = base::Bind(
      &XWalkPresentationServiceDelegateWin::OnSessionStarted,
      AsWeakPtr(), render_frame_host_id, success_cb, error_cb);
  PresentationSession::Create(params, callback);
}

void XWalkPresentationServiceDelegateWin::JoinSession(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_url,
    const std::string& presentation_id,
    const content::PresentationSessionStartedCallback& success_cb,
    const content::PresentationSessionErrorCallback& error_cb) {
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

void XWalkPresentationServiceDelegateWin::CloseConnection(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_id) {
  NOTIMPLEMENTED();
}

void XWalkPresentationServiceDelegateWin::Terminate(
    int render_process_id,
    int render_frame_id,
    const std::string& presentation_id) {
  RenderFrameHostId id(render_process_id, render_frame_id);
  auto presentation_frame = presentation_frames_.get(id);
  CHECK(presentation_frame);

  if (auto session = presentation_frame->session())
    session->Close();
}

void XWalkPresentationServiceDelegateWin::ListenForConnectionStateChange(
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

PresentationFrame* XWalkPresentationServiceDelegateWin::
    GetOrAddPresentationFrame(const RenderFrameHostId& render_frame_host_id) {
  if (!presentation_frames_.contains(render_frame_host_id)) {
    presentation_frames_.add(
        render_frame_host_id,
        scoped_ptr<PresentationFrame>(
            new PresentationFrame()));
  }
  return presentation_frames_.get(render_frame_host_id);
}

}  // namespace xwalk
