// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/nacl_host/nacl_browser_delegate_impl.h"

#include "base/path_service.h"
#include "content/public/browser/browser_thread.h"
#include "ppapi/c/private/ppb_nacl_private.h"
#include "xwalk/runtime/browser/renderer_host/pepper/xwalk_browser_pepper_host_factory.h"
#include "xwalk/runtime/common/xwalk_paths.h"

namespace {

// Calls OnKeepaliveOnUIThread on UI thread.
void OnKeepalive(
    const content::BrowserPpapiHost::OnKeepaliveInstanceData& instance_data,
    const base::FilePath& profile_data_directory) {
  DCHECK(!content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
}

}  // namespace

NaClBrowserDelegateImpl::NaClBrowserDelegateImpl() {
}

NaClBrowserDelegateImpl::~NaClBrowserDelegateImpl() {
}

void NaClBrowserDelegateImpl::ShowMissingArchInfobar(int render_process_id,
                                                     int render_view_id) {
}

bool NaClBrowserDelegateImpl::DialogsAreSuppressed() {
  return true;
}

bool NaClBrowserDelegateImpl::GetCacheDirectory(base::FilePath* cache_dir) {
  return PathService::Get(xwalk::DIR_DATA_PATH, cache_dir);
}

bool NaClBrowserDelegateImpl::GetPluginDirectory(base::FilePath* plugin_dir) {
  return PathService::Get(xwalk::DIR_INTERNAL_PLUGINS, plugin_dir);
}

bool NaClBrowserDelegateImpl::GetPnaclDirectory(base::FilePath* pnacl_dir) {
  return PathService::Get(xwalk::DIR_PNACL_COMPONENT, pnacl_dir);
}

bool NaClBrowserDelegateImpl::GetUserDirectory(base::FilePath* user_dir) {
  return PathService::Get(xwalk::DIR_DATA_PATH, user_dir);
}

std::string NaClBrowserDelegateImpl::GetVersionString() const {
  return "Chrome/" CHROME_VERSION " Crosswalk/" XWALK_VERSION;
}

ppapi::host::HostFactory* NaClBrowserDelegateImpl::CreatePpapiHostFactory(
    content::BrowserPpapiHost* ppapi_host) {
  return new xwalk::XWalkBrowserPepperHostFactory(ppapi_host);
}

void NaClBrowserDelegateImpl::SetDebugPatterns(std::string debug_patterns) {
}

bool NaClBrowserDelegateImpl::URLMatchesDebugPatterns(
    const GURL& manifest_url) {
  return false;
}

// This function is security sensitive.  Be sure to check with a security
// person before you modify it.
bool NaClBrowserDelegateImpl::MapUrlToLocalFilePath(
    const GURL& file_url, bool use_blocking_api, const base::FilePath& profile_directory, base::FilePath* file_path) {
  return false;
}

content::BrowserPpapiHost::OnKeepaliveCallback
NaClBrowserDelegateImpl::GetOnKeepaliveCallback() {
  return base::Bind(&OnKeepalive);
}
