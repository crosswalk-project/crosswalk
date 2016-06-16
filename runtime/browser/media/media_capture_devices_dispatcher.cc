// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"

#include "base/strings/utf_string_conversions.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/media_capture_devices.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/media_stream_request.h"
#include "grit/xwalk_resources.h"
#include "net/base/url_util.h"
#include "ui/base/l10n/l10n_util.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_content_settings.h"
#include "xwalk/runtime/common/xwalk_system_locale.h"

#if !defined(OS_ANDROID)
#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_dialog_manager.h"
#endif

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
}

}  // namespace

namespace xwalk {

XWalkMediaCaptureDevicesDispatcher*
    XWalkMediaCaptureDevicesDispatcher::GetInstance() {
  return base::Singleton<XWalkMediaCaptureDevicesDispatcher>::get();
}

bool ContentTypeIsRequested(ContentSettingsType type,
    const content::MediaStreamRequest& request) {
  if (type == CONTENT_SETTINGS_TYPE_MEDIASTREAM_MIC)
    return request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE;
  if (type == CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA)
    return request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE;

  return false;
}

int GetDialogMessageText(const content::MediaStreamRequest& request) {
  bool audio_requested =
      request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE;
  bool video_requested =
      request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE;
  if (audio_requested && video_requested)
    return IDS_MEDIA_CAPTURE_AUDIO_AND_VIDEO;
  if (video_requested)
    return IDS_MEDIA_CAPTURE_VIDEO_ONLY;

  return IDS_MEDIA_CAPTURE_AUDIO_ONLY;
}

void XWalkMediaCaptureDevicesDispatcher::RunRequestMediaAccessPermission(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  content::MediaStreamDevices devices;
  if (ContentTypeIsRequested(
          CONTENT_SETTINGS_TYPE_MEDIASTREAM_MIC, request) ||
      ContentTypeIsRequested(
          CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA, request)) {
    switch (request.request_type) {
      case content::MEDIA_OPEN_DEVICE_PEPPER_ONLY:
      case content::MEDIA_DEVICE_ACCESS:
      case content::MEDIA_GENERATE_STREAM:
      case content::MEDIA_ENUMERATE_DEVICES: {
#if defined (OS_ANDROID)
        // Get the exact audio and video devices if id is specified.
        // Or get the default devices when requested device id is empty.
        XWalkMediaCaptureDevicesDispatcher::GetInstance()->GetRequestedDevice(
            request.requested_audio_device_id,
            request.requested_video_device_id,
            request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE,
            request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE,
            &devices);
        break;
#else
        RequestPermissionToUser(web_contents, request, callback);
        break;
#endif
      }
    }
  }
#if defined (OS_ANDROID)
  callback.Run(devices,
               devices.empty() ?
                   content::MEDIA_DEVICE_NO_HARDWARE :
                   content::MEDIA_DEVICE_OK,
               std::unique_ptr<content::MediaStreamUI>());
#endif
}

#if !defined (OS_ANDROID)
void XWalkMediaCaptureDevicesDispatcher::RequestPermissionToUser(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  // This function may be called for a media request coming from
  // from WebRTC/mediaDevices. These requests can't be made from HTTP.
  if (request.security_origin.SchemeIs(url::kHttpScheme) &&
      !net::IsLocalhost(request.security_origin.host()))
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 std::unique_ptr<content::MediaStreamUI>());

  XWalkPermissionDialogManager* permission_dialog_manager =
      XWalkPermissionDialogManager::GetPermissionDialogManager(web_contents);

  XWalkBrowserContext* browser_context =
      static_cast<XWalkBrowserContext*>(web_contents->GetBrowserContext());

  PrefService* pref_service =
      user_prefs::UserPrefs::Get(browser_context);

  application::Application* app =
      browser_context->application_service()->GetApplicationByRenderHostID(
          web_contents->GetMainFrame()->GetProcess()->GetID());
  std::string app_name;
  if (app)
    app_name = app->data()->Name();

  ContentSettingsType content_settings_type =
      request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE
          ? CONTENT_SETTINGS_TYPE_MEDIASTREAM_MIC
          : CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA;

  base::string16 dialog_text = l10n_util::GetStringFUTF16(
      GetDialogMessageText(request),
      base::ASCIIToUTF16(app_name));

  permission_dialog_manager->RequestPermission(
      content_settings_type,
      request.security_origin,
      pref_service->GetString(kIntlAcceptLanguage), dialog_text,
      base::Bind(
          &XWalkMediaCaptureDevicesDispatcher::OnPermissionRequestFinished,
          callback, request, web_contents));
}

void XWalkMediaCaptureDevicesDispatcher::OnPermissionRequestFinished(
    const content::MediaResponseCallback& callback,
    const content::MediaStreamRequest& request,
    content::WebContents* web_contents,
    bool success) {
  content::MediaStreamDevices devices;
  bool audio_requested =
      request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE;
  bool video_requested =
      request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE;
  // We always request the permission for the audio capture if both audio and
  // video are requested (though the dialog clearly shows both),
  // let's make sure we set the video as well in the settings.
  if (audio_requested && video_requested) {
    XWalkContentSettings::GetInstance()->SetPermission(
      CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA,
      request.security_origin,
      web_contents->GetLastCommittedURL().GetOrigin(),
      success ? CONTENT_SETTING_ALLOW : CONTENT_SETTING_BLOCK);
  }
  if (success) {
    // Get the exact audio and video devices if id is specified.
    // Or get the default devices when requested device id is empty.
    XWalkMediaCaptureDevicesDispatcher::GetInstance()->GetRequestedDevice(
        request.requested_audio_device_id,
        request.requested_video_device_id,
        audio_requested,
        video_requested,
        &devices);
      callback.Run(devices,
                   devices.empty() ?
                      content::MEDIA_DEVICE_NO_HARDWARE :
                      content::MEDIA_DEVICE_OK,
                   std::unique_ptr<content::MediaStreamUI>());
  } else {
    callback.Run(devices,
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 std::unique_ptr<content::MediaStreamUI>());
  }
}
#endif

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
          &XWalkMediaCaptureDevicesDispatcher::NotifyAudioDevicesChangedOnUIThread, // NOLINT
          base::Unretained(this)));
}

void XWalkMediaCaptureDevicesDispatcher::OnVideoCaptureDevicesChanged() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &XWalkMediaCaptureDevicesDispatcher::NotifyVideoDevicesChangedOnUIThread, // NOLINT
          base::Unretained(this)));
}

void XWalkMediaCaptureDevicesDispatcher::OnMediaRequestStateChanged(
    int render_process_id,
    int render_frame_id,
    int page_request_id,
    const GURL& security_origin,
    content::MediaStreamType stream_type,
    content::MediaRequestState state) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &XWalkMediaCaptureDevicesDispatcher::UpdateMediaReqStateOnUIThread,
          base::Unretained(this), render_process_id, render_frame_id,
          security_origin, stream_type, state));
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
    int render_frame_id,
    const GURL& security_origin,
    content::MediaStreamType stream_type,
    content::MediaRequestState state) {
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnRequestUpdate(render_process_id,
                                    render_frame_id,
                                    stream_type,
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

}  // namespace xwalk
