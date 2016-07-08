// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_ANDROID_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_ANDROID_H_

#include "xwalk/runtime/browser/android/xwalk_presentation_host.h"
#include "xwalk/runtime/browser/xwalk_presentation_service_helper.h"

namespace xwalk {

class PresentationSessionAndroid : public PresentationSession {
 public:
  static void Create(const CreateParams& params, SessionCallback callback);
  ~PresentationSessionAndroid() override;
  void Close() override;

 protected:
  PresentationSessionAndroid(const std::string& presentation_url,
                             const std::string& presentation_id,
                             const SystemString& display_id);

  DISALLOW_COPY_AND_ASSIGN(PresentationSessionAndroid);
};

class DisplayInfoManagerServiceAndroid : public DisplayInfoManagerService {
 public:
  DisplayInfoManagerServiceAndroid();
  ~DisplayInfoManagerServiceAndroid() override;
  void FindAllAvailableMonitors(std::vector<DisplayInfo>* info_list) override;
  void ListenMonitorsUpdate() override;
  void StopListenMonitorsUpdate() override;

  static void DisplayChangeCallback(int display_id);
};

class PresentationFrameAndroid : public PresentationFrame,
                                 public XWalkPresentationHost::SessionObserver {
 public:
  explicit PresentationFrameAndroid(
      const RenderFrameHostId& render_frame_host_id);
  ~PresentationFrameAndroid() override;

  void OnPresentationClosed(int render_process_id, int render_frame_id);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PRESENTATION_SERVICE_HELPER_ANDROID_H_
