// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/tizen/tizen_locale_listener.h"

#include <vconf.h>
#include <algorithm>

#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "xwalk/runtime/browser/xwalk_runner_tizen.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"

namespace xwalk {
namespace {
const char kTizenDefaultLocale[] = "en-GB";
const char kTizenLocaleListenerThreadName[] = "TizenLocaleListener";

bool TizenLocaleToBCP47Locale(const std::string& tizen_locale,
                              std::string* out_BCP47_locale) {
  if (tizen_locale.empty())
    return false;

  // Tizen locale format [language[_territory][.codeset][@modifier]] .
  // Conver to BCP47 format language[-territory] .
  *out_BCP47_locale = tizen_locale.substr(0, tizen_locale.find_first_of("."));
  std::replace(out_BCP47_locale->begin(), out_BCP47_locale->end(), '_', '-');
  return true;
}

void OnVconfLangSetChanged(keynode_t *key, void *user_data) {
  if (vconf_keynode_get_type(key) != VCONF_TYPE_STRING)
    return;
  TizenLocaleListener* tizen_locale_listener =
      static_cast<TizenLocaleListener*>(user_data);

  std::string locale;
  if (TizenLocaleToBCP47Locale(vconf_keynode_get_str(key), &locale)) {
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
        base::Bind(&TizenLocaleListener::SetLocale,
                   base::Unretained(tizen_locale_listener),
                   locale));
  }
}

std::string GetSystemLocale() {
  std::string tizen_locale;
  char* langset = vconf_get_str(VCONFKEY_LANGSET);
  if (langset) {
    tizen_locale = langset;
  } else {
    LOG(ERROR) << "Can not get VCONFKEY_LANGSET from vconf or "
               << "VCONFKEY_LANGSET vlaue is not a string value";
  }
  free(langset);

  // Tizen take en-GB as default.
  std::string BCP47_locale(kTizenDefaultLocale);
  TizenLocaleToBCP47Locale(tizen_locale, &BCP47_locale);
  return BCP47_locale;
}

}  // namespace

TizenLocaleListener::TizenLocaleListener()
    : SimpleThread(kTizenLocaleListenerThreadName),
      locale_(GetSystemLocale()) {
  vconf_notify_key_changed(VCONFKEY_LANGSET, OnVconfLangSetChanged, this);
  main_loop_ = g_main_loop_new(NULL, FALSE);
  Start();
}

TizenLocaleListener::~TizenLocaleListener() {
  g_main_loop_quit(main_loop_);
  g_main_loop_unref(main_loop_);
  SimpleThread::Join();
  vconf_ignore_key_changed(VCONFKEY_LANGSET, OnVconfLangSetChanged);
}

void TizenLocaleListener::Run() {
  g_main_loop_run(main_loop_);
}

std::string TizenLocaleListener::GetLocale() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  // Note: we can only get locale from main thread for thread safe.
  return locale_;
}

void TizenLocaleListener::SetLocale(const std::string& locale) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  // Note: we can only set locale from main thread for thread safe.
  // If you need set locale from other thread, please PostTask to main thread.
  if (locale_ == locale)
    return;

  LOG(INFO) << "Locale change from " << locale_ << " to " << locale;
  locale_ = locale;

  application::ApplicationSystem* application_system_ =
      XWalkRunnerTizen::GetInstance()->app_system();
  if (application_system_)
    application_system_->application_service()->ChangeLocale(locale);
}

}  // namespace xwalk
