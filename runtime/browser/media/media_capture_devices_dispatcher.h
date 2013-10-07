// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_
#define XWALK_RUNTIME_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_

#include <string>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/singleton.h"
#include "base/observer_list.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/media_stream_request.h"

// This singleton is used to receive updates about media events from the content
// layer. Based on chrome/browser/media/media_capture_devices_dispatcher.[h|cc].
class XWalkMediaCaptureDevicesDispatcher : public content::MediaObserver {
 public:
  class Observer {
   public:
    // Handle an information update consisting of a up-to-date audio capture
    // device lists. This happens when a microphone is plugged in or unplugged.
    virtual void OnUpdateAudioDevices(
        const content::MediaStreamDevices& devices) {}

    // Handle an information update consisting of a up-to-date video capture
    // device lists. This happens when a camera is plugged in or unplugged.
    virtual void OnUpdateVideoDevices(
        const content::MediaStreamDevices& devices) {}

    // Handle an information update related to a media stream request.
    virtual void OnRequestUpdate(
        int render_process_id,
        int render_view_id,
        const content::MediaStreamDevice& device,
        const content::MediaRequestState state) {}

    virtual ~Observer() {}
  };

  static XWalkMediaCaptureDevicesDispatcher* GetInstance();

  static void RunRequestMediaAccessPermission(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback);

  // Methods for observers. Called on UI thread.
  // Observers should add themselves on construction and remove themselves
  // on destruction.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);
  const content::MediaStreamDevices& GetAudioCaptureDevices();
  const content::MediaStreamDevices& GetVideoCaptureDevices();

  // Helper for picking the device that was requested for an OpenDevice request.
  // If the device requested is not available it will revert to using the first
  // available one instead or will return an empty list if no devices of the
  // requested kind are present.
  void GetRequestedDevice(const std::string& requested_device_id,
                          bool audio,
                          bool video,
                          content::MediaStreamDevices* devices);

  // Overridden from content::MediaObserver:
  virtual void OnAudioCaptureDevicesChanged(
      const content::MediaStreamDevices& devices) OVERRIDE;
  virtual void OnVideoCaptureDevicesChanged(
      const content::MediaStreamDevices& devices) OVERRIDE;
  virtual void OnMediaRequestStateChanged(
      int render_process_id,
      int render_view_id,
      int page_request_id,
      const content::MediaStreamDevice& device,
      content::MediaRequestState state) OVERRIDE;
  virtual void OnAudioStreamPlayingChanged(
      int render_process_id,
      int render_view_id,
      int stream_id,
      bool is_playing,
      float power_dbfs,
      bool clipped) OVERRIDE {}
  virtual void OnCreatingAudioStream(int render_process_id,
                                     int render_view_id) OVERRIDE {}

 private:
  friend struct DefaultSingletonTraits<XWalkMediaCaptureDevicesDispatcher>;

  XWalkMediaCaptureDevicesDispatcher();
  virtual ~XWalkMediaCaptureDevicesDispatcher();

  // Called by the MediaObserver() functions, executed on UI thread.
  void UpdateAudioDevicesOnUIThread(const content::MediaStreamDevices& devices);
  void UpdateVideoDevicesOnUIThread(const content::MediaStreamDevices& devices);
  void UpdateMediaReqStateOnUIThread(
      int render_process_id,
      int render_view_id,
      const content::MediaStreamDevice& device,
      content::MediaRequestState state);

  // A list of cached audio capture devices.
  content::MediaStreamDevices audio_devices_;

  // A list of cached video capture devices.
  content::MediaStreamDevices video_devices_;

  // A list of observers for the device update notifications.
  ObserverList<Observer> observers_;

  // Flag to indicate if device enumeration has been done/doing.
  // Only accessed on UI thread.
  bool devices_enumerated_;
};

#endif  // XWALK_RUNTIME_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_
