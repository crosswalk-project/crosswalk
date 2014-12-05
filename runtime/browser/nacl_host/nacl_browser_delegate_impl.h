// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_NACL_HOST_NACL_BROWSER_DELEGATE_IMPL_H_
#define XWALK_RUNTIME_BROWSER_NACL_HOST_NACL_BROWSER_DELEGATE_IMPL_H_

#include <string>

#include "components/nacl/browser/nacl_browser_delegate.h"

class NaClBrowserDelegateImpl : public NaClBrowserDelegate {
 public:
  NaClBrowserDelegateImpl();
  virtual ~NaClBrowserDelegateImpl();

  void ShowMissingArchInfobar(int render_process_id,
                              int render_view_id) override;
  bool DialogsAreSuppressed() override;
  bool GetCacheDirectory(base::FilePath* cache_dir) override;
  bool GetPluginDirectory(base::FilePath* plugin_dir) override;
  bool GetPnaclDirectory(base::FilePath* pnacl_dir) override;
  bool GetUserDirectory(base::FilePath* user_dir) override;
  std::string GetVersionString() const override;
  ppapi::host::HostFactory* CreatePpapiHostFactory(
      content::BrowserPpapiHost* ppapi_host) override;
  bool MapUrlToLocalFilePath(const GURL& url,
                             bool is_blocking,
                             const base::FilePath& profile_directory,
                             base::FilePath* file_path) override;
  void SetDebugPatterns(std::string debug_patterns) override;
  bool IsNonSfiModeAllowed(const base::FilePath& profile_directory,
                           const GURL& manifest_url) override;
  bool URLMatchesDebugPatterns(const GURL& manifest_url) override;
  content::BrowserPpapiHost::OnKeepaliveCallback
      GetOnKeepaliveCallback() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NaClBrowserDelegateImpl);
};

#endif  // XWALK_RUNTIME_BROWSER_NACL_HOST_NACL_BROWSER_DELEGATE_IMPL_H_
