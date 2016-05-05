// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_WIN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_WIN_H_

#include "xwalk/runtime/browser/xwalk_presentation_service_helper.h"

namespace xwalk {

class PresentationSessionWin : public PresentationSession,
                               public Runtime::Observer {
 public:
  static void Create(const CreateParams& params, SessionCallback callback);
  ~PresentationSessionWin() override;
  void Close() override;

 protected:
  PresentationSessionWin(const std::string& presentation_url,
                         const std::string& presentation_id,
                         const SystemString& display_id);

 private:
  void OnNewRuntimeAdded(Runtime* new_runtime) override;
  void OnRuntimeClosed(Runtime* runtime) override;
  void OnApplicationExitRequested(Runtime* runtime) override;

  ScopedVector<Runtime> runtimes_;

  DISALLOW_COPY_AND_ASSIGN(PresentationSessionWin);
};

class DisplayInfoManagerServiceWin : public DisplayInfoManagerService {
 public:
  DisplayInfoManagerServiceWin();
  ~DisplayInfoManagerServiceWin() override;
  void FindAllAvailableMonitors(std::vector<DisplayInfo>* info_list) override;
  void ListenMonitorsUpdate() override;
  void StopListenMonitorsUpdate() override;

  static BOOL CALLBACK MonitorEnumCallback(HMONITOR hMonitor,
                                           HDC hdc,
                                           LPRECT lprcMonitor,
                                           LPARAM lParam);

  static LRESULT CALLBACK WndProcCallback(HWND hWnd,
                                          UINT message,
                                          WPARAM wParam,
                                          LPARAM lParam);

 private:
  HWND hwnd_;
  std::vector<DisplayInfo>* info_list_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_WIN_H_
