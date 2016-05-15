// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_autofill_manager.h"

#include <string>

#include "components/prefs/pref_service.h"
#include "components/autofill/content/browser/content_autofill_driver_factory.h"
#include "components/autofill/core/browser/autofill_manager.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/renderer_preferences.h"
#include "xwalk/runtime/browser/xwalk_autofill_client.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_system_locale.h"

#if defined(OS_ANDROID)
#include "base/android/locale_utils.h"
#include "xwalk/runtime/browser/android/xwalk_autofill_client_android.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"
#endif

#if !defined(OS_ANDROID)
#include "xwalk/runtime/browser/xwalk_autofill_client_desktop.h"
#endif

namespace xwalk {

XWalkAutofillManager::~XWalkAutofillManager() {
}

XWalkAutofillManager::XWalkAutofillManager(
    content::WebContents* web_contents)
    : web_contents_(web_contents) {
#if defined(OS_ANDROID)
  XWalkAutofillClient* autofill_manager_delegate =
      XWalkAutofillClientAndroid::FromWebContents(web_contents_);
  if (autofill_manager_delegate)
    InitAutofillIfNecessary(autofill_manager_delegate->GetSaveFormData());
#else
  InitAutofillIfNecessary(true);
#endif

  CreateUserPrefServiceIfNecessary();
  PrefService* pref_service =
    user_prefs::UserPrefs::Get(XWalkBrowserContext::GetDefault());
  pref_change_registrar_.Init(pref_service);
  if (pref_service) {
    base::Closure renderer_callback = base::Bind(
      &XWalkAutofillManager::UpdateRendererPreferences, base::Unretained(this));
    pref_change_registrar_.Add(kIntlAcceptLanguage, renderer_callback);
  }
}

void XWalkAutofillManager::UpdateRendererPreferences() {
  content::RendererPreferences* prefs =
    web_contents_->GetMutableRendererPrefs();
  PrefService* pref_service =
    user_prefs::UserPrefs::Get(XWalkBrowserContext::GetDefault());
  const std::string& accept_languages =
    pref_service->GetString(kIntlAcceptLanguage);
  prefs->accept_languages = accept_languages;
  web_contents_->GetRenderViewHost()->SyncRendererPrefs();
  XWalkBrowserContext* browser_context =
      XWalkBrowserContext::FromWebContents(web_contents_);
  CHECK(browser_context);
  browser_context->UpdateAcceptLanguages(accept_languages);
}

void XWalkAutofillManager::InitAutofillIfNecessary(bool enabled) {
  // Do not initialize if the feature is not enabled.
  if (!enabled)
    return;
  // Check if the autofill driver factory already exists.
  if (autofill::ContentAutofillDriverFactory::FromWebContents(web_contents_))
    return;

  CreateUserPrefServiceIfNecessary();
#if defined (OS_ANDROID)
  XWalkAutofillClientAndroid::CreateForWebContents(web_contents_);
  autofill::ContentAutofillDriverFactory::CreateForWebContentsAndDelegate(
      web_contents_,
      XWalkAutofillClientAndroid::FromWebContents(web_contents_),
      base::android::GetDefaultLocale(),
      autofill::AutofillManager::DISABLE_AUTOFILL_DOWNLOAD_MANAGER);
#else
  XWalkAutofillClientDesktop::CreateForWebContents(web_contents_);
  autofill::ContentAutofillDriverFactory::CreateForWebContentsAndDelegate(
      web_contents_,
      XWalkAutofillClientDesktop::FromWebContents(web_contents_),
      XWalkContentBrowserClient::Get()->GetApplicationLocale(),
      autofill::AutofillManager::DISABLE_AUTOFILL_DOWNLOAD_MANAGER);
#endif
}

void XWalkAutofillManager::CreateUserPrefServiceIfNecessary() {
  XWalkBrowserContext* browser_context =
      XWalkBrowserContext::FromWebContents(web_contents_);
  browser_context->CreateUserPrefServiceIfNecessary();
}

}  // namespace xwalk
