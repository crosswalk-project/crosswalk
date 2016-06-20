// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner_win.h"

#include "base/memory/ptr_util.h"
#include "base/win/registry.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/runtime/browser/wifidirect_component_win.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"

namespace xwalk {

namespace {

const auto g_google_api = L"GOOGLE_API_KEY";
const auto g_google_default_client_id = L"GOOGLE_DEFAULT_CLIENT_ID";
const auto g_google_default_client_secret = L"GOOGLE_DEFAULT_CLIENT_SECRET";

}  // namespace

XWalkRunnerWin::XWalkRunnerWin() {
}

void XWalkRunnerWin::CreateComponents() {
  XWalkRunner::CreateComponents();
  if (XWalkRuntimeFeatures::isWiFiDirectAPIEnabled())
    AddComponent(base::WrapUnique(new WiFiDirectComponent()));
}

void XWalkRunnerWin::InitializeEnvironmentVariablesForGoogleAPIs(
  content::RenderProcessHost* host) {
  application::Application* app =
      app_system()->application_service()->
          GetApplicationByRenderHostID(host->GetID());
  if (!app)
    return;
  std::string app_name;
  if (!app->data()->GetManifest()->GetString(
          application_manifest_keys::kXWalkPackageId, &app_name))
    return;
  std::string vendor =
      app_name.substr(0, app_name.find_last_of("."));
  std::wstringstream registry_path;
  registry_path << L"SOFTWARE\\" << vendor.c_str() << L"\\"
                << app_name.c_str();
  base::win::RegKey key(HKEY_CURRENT_USER,
                        registry_path.str().c_str(),
                        KEY_READ);
  std::wstring google_api_key;
  if (key.ReadValue(g_google_api,
                    &google_api_key) != ERROR_SUCCESS)
    return;
  std::wstring google_default_client_id;
  if (key.ReadValue(g_google_default_client_id,
                    &google_default_client_id) != ERROR_SUCCESS)
    return;
  std::wstring google_default_client_secret;
  if (key.ReadValue(g_google_default_client_secret,
                    &google_default_client_secret) != ERROR_SUCCESS)
    return;
  SetEnvironmentVariable(g_google_api,
                         google_api_key.c_str());
  SetEnvironmentVariable(g_google_default_client_id,
                         google_default_client_id.c_str());
  SetEnvironmentVariable(g_google_default_client_secret,
                         google_default_client_secret.c_str());
}

}  // namespace xwalk
