// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_geolocation_permission_context.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_content.h"
#endif

#if defined(OS_TIZEN)
#include <cynara-client.h>
#include <cynara-creds-socket.h>
// #include <sockets/SocketManager.h>
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/permissions_handler.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/browser/xwalk_runner_tizen.h"
#endif

namespace xwalk {

void CancelGeolocationPermissionRequestOnUIThread(
        int render_process_id,
        int render_view_id,
        const GURL& requesting_frame) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

#if defined(OS_ANDROID)
  XWalkContent* xwalk_content =
      XWalkContent::FromID(render_process_id, render_view_id);
  if (xwalk_content) {
      xwalk_content->HideGeolocationPrompt(requesting_frame);
  }
#endif
  // TODO(yongsheng): Handle this for other platforms.
}

void CancelGeolocationPermissionRequest(
    int render_process_id,
    int render_view_id,
    const GURL& requesting_frame) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(
          &CancelGeolocationPermissionRequestOnUIThread,
          render_process_id,
          render_view_id,
          requesting_frame));
}

#if defined(OS_TIZEN)
void CheckCynara(base::Callback<void(bool)> result_callback) {
  // check cynara synchrounously here
  char* client_id;
  char* user_id;
  int socket_fd = 1;  // FIXME: to be replaced with appropriate socket
  // int socket_fd = SocketManager::getSocketFromSystemD(
  //        PathConfig::SocketPath::client);

  int ret_client  = cynara_creds_socket_get_client(socket_fd,
          CLIENT_METHOD_SMACK, &client_id);
  if (ret_client != CYNARA_API_SUCCESS) {
    LOG(ERROR) << "cynara failed to get client_id, error_code " + ret_client;
    return;
  }
  int ret_user = cynara_creds_socket_get_user(socket_fd,
          USER_METHOD_UID, &user_id);
  if (ret_user != CYNARA_API_SUCCESS) {
    LOG(ERROR) << "cynara failed to get user_id, error code " + ret_user;
    free(client_id);
    return;
  }


  XWalkRunnerTizen* runner_tizen = XWalkRunnerTizen::GetInstance();
  int ret = cynara_check(runner_tizen->GetCynara(),
      client_id, "", user_id,
      "http://tizen.org/privilege/location");
  // Note: empty string here is client_session, currently not used

  switch (ret) {
    // TODO(terriko): CYNARA_API_SUCCESS will be changed to
    //                CYNARA_API_ACCESS_ALLOWED in new cynara
    case CYNARA_API_SUCCESS:
      result_callback.Run(true);
      break;
    case CYNARA_API_ACCESS_DENIED:
      LOG(WARNING) << "cynara geolocation check denied";
      break;
    default:
      LOG(ERROR) << "cynara geolocation check returned unexpected reponse";
  }
  free(client_id);
  free(user_id);
}
#endif
RuntimeGeolocationPermissionContext::~RuntimeGeolocationPermissionContext() {
}

RuntimeGeolocationPermissionContext::RuntimeGeolocationPermissionContext() {
}

void
RuntimeGeolocationPermissionContext::RequestGeolocationPermissionOnUIThread(
    content::WebContents* web_contents,
    const GURL& requesting_frame,
    base::Callback<void(bool)> result_callback,
    base::Closure* cancel_callback) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  int render_process_id = web_contents->GetRenderProcessHost()->GetID();
  int render_view_id = web_contents->GetRenderViewHost()->GetRoutingID();

#if defined(OS_ANDROID)
  XWalkContent* xwalk_content =
      XWalkContent::FromID(render_process_id, render_view_id);
  if (!xwalk_content) {
    result_callback.Run(false);
    return;
  }

  xwalk_content->ShowGeolocationPrompt(requesting_frame, result_callback);
#elif defined(OS_TIZEN)
#if !defined(CYNARA)
  bool has_geolocation_permission = false;
  XWalkRunner* runner = XWalkRunner::GetInstance();
  application::ApplicationSystem* app_system = runner->app_system();
  application::ApplicationService* app_service =
      app_system->application_service();
  application::Application* application =
      app_service->GetApplicationByRenderHostID(render_view_id);

  if (application) {
    DCHECK(application->data());
    application::PermissionsInfo* info =
      static_cast<application::PermissionsInfo*>(
      application->data()->GetManifestData(
          application_manifest_keys::kPermissionsKey));

    if (info) {
      const application::PermissionSet& permissions = info->GetAPIPermissions();
      application::PermissionSet::const_iterator it =
          std::find(permissions.begin(), permissions.end(), "geolocation");
      has_geolocation_permission = it != permissions.end();
    }
  }

  result_callback.Run(has_geolocation_permission);
#else
  if (!thread_.get()) {
    thread_.reset(new base::Thread("cynara_geolocation"));
  }
  thread_->message_loop()->PostTask(
      FROM_HERE, base::Bind(CheckCynara, result_callback));
  thread_->Start();
#endif

#endif

  if (cancel_callback) {
     *cancel_callback = base::Bind(
         CancelGeolocationPermissionRequest,
         render_process_id,
         render_view_id,
         requesting_frame);
  }

  // TODO(yongsheng): Handle this for other platforms.
}

void
RuntimeGeolocationPermissionContext::RequestGeolocationPermission(
    content::WebContents* web_contents,
    const GURL& requesting_frame,
    base::Callback<void(bool)> result_callback,
    base::Closure* cancel_callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(
          &RuntimeGeolocationPermissionContext::
              RequestGeolocationPermissionOnUIThread,
          this,
          web_contents,
          requesting_frame,
          result_callback,
          cancel_callback));
}

}  // namespace xwalk
