// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_presentation_service_helper_win.h"

namespace xwalk {

DisplayInfoManagerServiceWin::DisplayInfoManagerServiceWin()
    : info_list_(nullptr), hwnd_(NULL) {}

DisplayInfoManagerServiceWin::~DisplayInfoManagerServiceWin() {}

void DisplayInfoManagerServiceWin::FindAllAvailableMonitors(
    std::vector<DisplayInfo>* info_list) {
  info_list_ = info_list;
  EnumDisplayMonitors(0, 0, MonitorEnumCallback,
                      reinterpret_cast<LPARAM>(this));
}

void DisplayInfoManagerServiceWin::ListenMonitorsUpdate() {
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

  hwnd_ = CreateWindowEx(0, class_name, NULL, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
                         HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);
  if (!hwnd_)
    LOG(ERROR) << "Failed to register a window for listening WM_DISPLAYCHANGE";
}

void DisplayInfoManagerServiceWin::StopListenMonitorsUpdate() {
  if (hwnd_) {
    CloseWindow(hwnd_);
    hwnd_ = NULL;
  }
}

BOOL DisplayInfoManagerServiceWin::MonitorEnumCallback(HMONITOR hMonitor,
                                                       HDC hdc,
                                                       LPRECT lprcMonitor,
                                                       LPARAM lParam) {
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

  auto self = reinterpret_cast<DisplayInfoManagerServiceWin*>(lParam);
  self->info_list_->push_back(info);

  return TRUE;
}

LRESULT DisplayInfoManagerServiceWin::WndProcCallback(HWND hWnd,
                                                      UINT message,
                                                      WPARAM wParam,
                                                      LPARAM lParam) {
  if (message == WM_DISPLAYCHANGE) {
    DisplayInfoManager::GetInstance()->UpdateInfoList();
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

PresentationSessionWin::PresentationSessionWin(
    const std::string& presentation_url,
    const std::string& presentation_id,
    const SystemString& display_id)
    : PresentationSession(presentation_url, presentation_id, display_id) {}

PresentationSessionWin::~PresentationSessionWin() {
  Close();
}

void PresentationSessionWin::Create(
    const PresentationSession::CreateParams& params,
    PresentationSession::SessionCallback callback) {
  scoped_refptr<PresentationSessionWin> session(new PresentationSessionWin(
      params.presentation_url, params.presentation_id, params.display_info.id));
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

void PresentationSessionWin::Close() {
  std::vector<Runtime*> to_be_closed(runtimes_.get());
  for (Runtime* runtime : to_be_closed)
    runtime->Close();
}

void PresentationSessionWin::OnNewRuntimeAdded(Runtime* runtime) {
  runtimes_.push_back(runtime);
  runtime->set_observer(this);
  // TODO(Mikhail): handle show popups in presentation context.
}

void PresentationSessionWin::OnRuntimeClosed(Runtime* runtime) {
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

void PresentationSessionWin::OnApplicationExitRequested(Runtime* runtime) {
}

}  // namespace xwalk
