// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_CONTEXT_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_CONTEXT_H_

#if defined(OS_ANDROID)
#include <string>
#endif

#include <map>
#include <vector>

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "components/visitedlink/browser/visitedlink_delegate.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "xwalk/runtime/browser/xwalk_form_database_service.h"
#include "xwalk/runtime/browser/xwalk_ssl_host_state_delegate.h"

#if defined(OS_ANDROID)
#include "base/strings/string_split.h"
#endif

namespace net {
class URLRequestContextGetter;
}

namespace content {
class DownloadManagerDelegate;
class PermissionManager;
}

namespace visitedlink {
class VisitedLinkMaster;
}

class PrefService;

namespace xwalk {

class RuntimeDownloadManagerDelegate;
class RuntimeURLRequestContextGetter;

namespace application {
class ApplicationService;
}

class XWalkBrowserContext
    : public content::BrowserContext,
      public visitedlink::VisitedLinkDelegate
{
 public:
  XWalkBrowserContext();
  ~XWalkBrowserContext() override;

  // Currently only one instance per process is supported.
  static XWalkBrowserContext* GetDefault();

  // Convenience method to returns the XWalkBrowserContext corresponding to the
  // given WebContents.
  static XWalkBrowserContext* FromWebContents(
      content::WebContents* web_contents);

  // BrowserContext implementation.
  scoped_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
      const base::FilePath& partition_path) override;
  base::FilePath GetPath() const override;
  bool IsOffTheRecord() const override;
  content::DownloadManagerDelegate* GetDownloadManagerDelegate() override;
  net::URLRequestContextGetter* GetRequestContext() override;
  net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) override;
  net::URLRequestContextGetter* GetMediaRequestContext() override;
  net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) override;
  net::URLRequestContextGetter* GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path,
          bool in_memory) override;
  content::ResourceContext* GetResourceContext() override;
  content::BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  content::PushMessagingService* GetPushMessagingService() override;
  content::SSLHostStateDelegate* GetSSLHostStateDelegate() override;
  content::PermissionManager* GetPermissionManager() override;
  content::BackgroundSyncController* GetBackgroundSyncController() override;

  RuntimeURLRequestContextGetter* GetURLRequestContextGetterById(
      const std::string& pkg_id);
  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors);
  net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors);
  void InitFormDatabaseService();
  XWalkFormDatabaseService* GetFormDatabaseService();
  void CreateUserPrefServiceIfNecessary();
  void UpdateAcceptLanguages(const std::string& accept_languages);
  void set_save_form_data(bool enable) { save_form_data_ = enable; }
  bool save_form_data() const { return save_form_data_; }
  application::ApplicationService* application_service() const {
    return application_service_;
  }
  void set_application_service(
      application::ApplicationService* application_service) {
    application_service_ = application_service;
  }
#if defined(OS_ANDROID)
  void SetCSPString(const std::string& csp);
  std::string GetCSPString() const;
#endif
  // These methods map to Add methods in visitedlink::VisitedLinkMaster.
  void AddVisitedURLs(const std::vector<GURL>& urls);
  // visitedlink::VisitedLinkDelegate implementation.
  void RebuildTable(
      const scoped_refptr<URLEnumerator>& enumerator) override;

 private:
  class RuntimeResourceContext;

  // Performs initialization of the XWalkBrowserContext while IO is still
  // allowed on the current thread.
  void InitWhileIOAllowed();

  // Reset visitedlink master and initialize it.
  void InitVisitedLinkMaster();

  application::ApplicationService* application_service_;
  scoped_ptr<RuntimeResourceContext> resource_context_;
  scoped_refptr<RuntimeDownloadManagerDelegate> download_manager_delegate_;
  scoped_refptr<RuntimeURLRequestContextGetter> url_request_getter_;
  scoped_ptr<PrefService> user_pref_service_;
  scoped_ptr<XWalkFormDatabaseService> form_database_service_;
  bool save_form_data_;
#if defined(OS_ANDROID)
  std::string csp_;
#endif
  scoped_ptr<visitedlink::VisitedLinkMaster> visitedlink_master_;

  typedef std::map<base::FilePath::StringType,
      scoped_refptr<RuntimeURLRequestContextGetter> >
      PartitionPathContextGetterMap;
  PartitionPathContextGetterMap context_getters_;
  scoped_ptr<XWalkSSLHostStateDelegate> ssl_host_state_delegate_;
  scoped_ptr<content::PermissionManager> permission_manager_;

  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserContext);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_CONTEXT_H_
