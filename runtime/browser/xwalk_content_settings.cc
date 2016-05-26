// Copyright 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_content_settings.h"

#include <string>

#include "base/base_paths.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "components/prefs/pref_filter.h"
#include "components/prefs/pref_service_factory.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "content/public/browser/browser_thread.h"
#include "url/gurl.h"
#include "xwalk/runtime/common/xwalk_paths.h"

namespace xwalk {

// static
XWalkContentSettings* XWalkContentSettings::GetInstance() {
  return base::Singleton<XWalkContentSettings>::get();
}

base::FilePath XWalkContentSettings::GetPrefFilePathFromPath(
    const base::FilePath& path) {
  return path.Append(FILE_PATH_LITERAL("Preferences"));
}

XWalkContentSettings::XWalkContentSettings() {
}

XWalkContentSettings::~XWalkContentSettings() {
}

void XWalkContentSettings::Init() {
  ContentSettingsPattern::SetNonWildcardDomainNonPortScheme("app");
  base::FilePath xwalk_data_dir;
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &xwalk_data_dir));
  sequenced_task_runner_ = JsonPrefStore::GetTaskRunnerForFile(
      xwalk_data_dir, content::BrowserThread::GetBlockingPool());

  pref_store_ = new JsonPrefStore(GetPrefFilePathFromPath(xwalk_data_dir),
      sequenced_task_runner_.get(),
      std::unique_ptr<PrefFilter>());

  // The name is misleading, we do not sync anything.
  pref_registry_ = new user_prefs::PrefRegistrySyncable();
  PrefServiceFactory pref_service_factory;
  pref_service_factory.set_user_prefs(pref_store_);

  pref_service_ = pref_service_factory.Create(pref_registry_.get());

  HostContentSettingsMap::RegisterProfilePrefs(pref_registry_.get());
  host_content_settings_map_ =
      new HostContentSettingsMap(pref_service_.get(), false, false);
}

void XWalkContentSettings::Shutdown() {
  pref_store_->CommitPendingWrite();
  host_content_settings_map_->ShutdownOnUIThread();
}

ContentSetting XWalkContentSettings::GetPermission(
    ContentSettingsType type,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  return host_content_settings_map_->GetContentSetting(
      requesting_origin,
      embedding_origin,
      type, std::string());
}

void XWalkContentSettings::SetPermission(
    ContentSettingsType type,
    const GURL& requesting_origin,
    const GURL& embedding_origin,
    ContentSetting content_setting) {
DCHECK_EQ(requesting_origin, requesting_origin.GetOrigin());
DCHECK_EQ(embedding_origin, embedding_origin.GetOrigin());
DCHECK(content_setting == CONTENT_SETTING_ALLOW ||
    content_setting == CONTENT_SETTING_BLOCK);

host_content_settings_map_->SetContentSettingCustomScope(
    ContentSettingsPattern::FromURLNoWildcard(requesting_origin),
    ContentSettingsPattern::FromURLNoWildcard(embedding_origin),
    type, std::string(), content_setting);
}

}  // namespace xwalk
