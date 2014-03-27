// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"

#include "content/public/browser/media_capture_devices.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/media_stream_request.h"

using content::BrowserThread;
using content::MediaCaptureDevices;
using content::MediaStreamDevices;

namespace {

const content::MediaStreamDevice* FindDefaultDeviceWithId(
    const content::MediaStreamDevices& devices,
    const std::string& device_id) {
  if (devices.empty()) {
    return NULL;
  }
  content::MediaStreamDevices::const_iterator iter = devices.begin();
  for (; iter != devices.end(); ++iter) {
    if (iter->id == device_id) {
      return &(*iter);
    }
  }

  return &(*devices.begin());
};

}  // namespace


XWalkMediaCaptureDevicesDispatcher*
    XWalkMediaCaptureDevicesDispatcher::GetInstance() {
  return Singleton<XWalkMediaCaptureDevicesDispatcher>::get();
}

void XWalkMediaCaptureDevicesDispatcher::RunRequestMediaAccessPermission(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  content::MediaStreamDevices devices;
  // Based on chrome/browser/media/media_stream_devices_controller.cc.
  bool microphone_requested =
      (request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE);
  bool webcam_requested =
      (request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE);
  if (microphone_requested || webcam_requested) {
    switch (request.request_type) {
      case content::MEDIA_OPEN_DEVICE:
      case content::MEDIA_DEVICE_ACCESS:
      case content::MEDIA_GENERATE_STREAM:
      case content::MEDIA_ENUMERATE_DEVICES:
        // Get the exact audio and video devices if id is specified.
        // Or get the default devices when requested device id is empty.
        XWalkMediaCaptureDevicesDispatcher::GetInstance()->GetRequestedDevice(
            request.requested_audio_device_id,
            request.requested_video_device_id,
            microphone_requested,
            webcam_requested,
            &devices);
        break;
    }
  }
  callback.Run(devices,
               content::MEDIA_DEVICE_OK,
               scoped_ptr<content::MediaStreamUI>());
}

XWalkMediaCaptureDevicesDispatcher::XWalkMediaCaptureDevicesDispatcher() {}

XWalkMediaCaptureDevicesDispatcher::~XWalkMediaCaptureDevicesDispatcher() {}

void XWalkMediaCaptureDevicesDispatcher::AddObserver(Observer* observer) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (!observers_.HasObserver(observer))
    observers_.AddObserver(observer);
}

void XWalkMediaCaptureDevicesDispatcher::RemoveObserver(Observer* observer) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  observers_.RemoveObserver(observer);
}

const MediaStreamDevices&
XWalkMediaCaptureDevicesDispatcher::GetAudioCaptureDevices() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (!test_audio_devices_.empty())
    return test_audio_devices_;

  return MediaCaptureDevices::GetInstance()->GetAudioCaptureDevices();
}

const MediaStreamDevices&
XWalkMediaCaptureDevicesDispatcher::GetVideoCaptureDevices() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (!test_video_devices_.empty())
    return test_video_devices_;

  return MediaCaptureDevices::GetInstance()->GetVideoCaptureDevices();
}

void XWalkMediaCaptureDevicesDispatcher::GetRequestedDevice(
    const std::string& requested_audio_device_id,
    const std::string& requested_video_device_id,
    bool audio,
    bool video,
    content::MediaStreamDevices* devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DCHECK(audio || video);

  if (audio) {
    const content::MediaStreamDevices& audio_devices = GetAudioCaptureDevices();
    const content::MediaStreamDevice* const device =
        FindDefaultDeviceWithId(audio_devices, requested_audio_device_id);
    if (device)
      devices->push_back(*device);
  }
  if (video) {
    const content::MediaStreamDevices& video_devices = GetVideoCaptureDevices();
    const content::MediaStreamDevice* const device =
        FindDefaultDeviceWithId(video_devices, requested_video_device_id);
    if (device)
      devices->push_back(*device);
  }
}

void XWalkMediaCaptureDevicesDispatcher::OnAudioCaptureDevicesChanged() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &XWalkMediaCaptureDevicesDispatcher::NotifyAudioDevicesChangedOnUIThread,
          base::Unretained(this)));
}

void XWalkMediaCaptureDevicesDispatcher::OnVideoCaptureDevicesChanged() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &XWalkMediaCaptureDevicesDispatcher::NotifyVideoDevicesChangedOnUIThread,
          base::Unretained(this)));
}

void XWalkMediaCaptureDevicesDispatcher::OnMediaRequestStateChanged(
    int render_process_id,
    int render_view_id,
    int page_request_id,
    const GURL& security_origin,
    const content::MediaStreamDevice& device,
    content::MediaRequestState state) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &XWalkMediaCaptureDevicesDispatcher::UpdateMediaReqStateOnUIThread,
          base::Unretained(this), render_process_id, render_view_id,
          security_origin, device, state));
}

void XWalkMediaCaptureDevicesDispatcher::NotifyAudioDevicesChangedOnUIThread() {
  MediaStreamDevices devices = GetAudioCaptureDevices();
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnUpdateAudioDevices(devices));
}

void XWalkMediaCaptureDevicesDispatcher::NotifyVideoDevicesChangedOnUIThread() {
  MediaStreamDevices devices = GetVideoCaptureDevices();
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnUpdateVideoDevices(devices));
}

void XWalkMediaCaptureDevicesDispatcher::UpdateMediaReqStateOnUIThread(
    int render_process_id,
    int render_view_id,
    const GURL& security_origin,
    const content::MediaStreamDevice& device,
    content::MediaRequestState state) {
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnRequestUpdate(render_process_id,
                                    render_view_id,
                                    device,
                                    state));
}


void XWalkMediaCaptureDevicesDispatcher::SetTestAudioCaptureDevices(
    const MediaStreamDevices& devices) {
  test_audio_devices_ = devices;
}

void XWalkMediaCaptureDevicesDispatcher::SetTestVideoCaptureDevices(
    const MediaStreamDevices& devices) {
  test_video_devices_ = devices;
}
