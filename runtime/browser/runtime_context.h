// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_CONTEXT_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_CONTEXT_H_

#if defined(OS_ANDROID)
#include <string>
#endif

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/geolocation_permission_context.h"

namespace net {
class URLRequestContextGetter;
}

namespace content {
class DownloadManagerDelegate;
}

namespace xwalk {

class RuntimeDownloadManagerDelegate;
class RuntimeURLRequestContextGetter;

class RuntimeContext : public content::BrowserContext {
 public:
  RuntimeContext();
  virtual ~RuntimeContext();

  // Convenience method to returns the RuntimeContext corresponding to the
  // given WebContents.
  static RuntimeContext* FromWebContents(content::WebContents* web_contents);

  // BrowserContext implementation.
  virtual base::FilePath GetPath() const OVERRIDE;
  virtual bool IsOffTheRecord() const OVERRIDE;
  virtual content::DownloadManagerDelegate*
      GetDownloadManagerDelegate() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter*
      GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path,
          bool in_memory) OVERRIDE;
  virtual content::ResourceContext* GetResourceContext() OVERRIDE;
  virtual content::GeolocationPermissionContext*
      GetGeolocationPermissionContext() OVERRIDE;
  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;
  virtual void RequestMidiSysExPermission(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame,
      const MidiSysExPermissionCallback& callback) OVERRIDE { }
  virtual void CancelMidiSysExPermissionRequest(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame) OVERRIDE {}
  virtual void RequestProtectedMediaIdentifierPermission(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      int group_id,
      const GURL& requesting_frame,
      const ProtectedMediaIdentifierPermissionCallback& callback) OVERRIDE;
  virtual void CancelProtectedMediaIdentifierPermissionRequests(
      int group_id) OVERRIDE;

  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers);
  net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers);
#if defined(OS_ANDROID)
  void SetCSPString(const std::string& csp);
  std::string GetCSPString() const;
#endif

 private:
  class RuntimeResourceContext;

  // Performs initialization of the RuntimeContext while IO is still
  // allowed on the current thread.
  void InitWhileIOAllowed();

  scoped_ptr<RuntimeResourceContext> resource_context_;
  scoped_refptr<RuntimeDownloadManagerDelegate> download_manager_delegate_;
  scoped_refptr<RuntimeURLRequestContextGetter> url_request_getter_;
  scoped_refptr<content::GeolocationPermissionContext>
       geolocation_permission_context_;
#if defined(OS_ANDROID)
  std::string csp_;
#endif
  DISALLOW_COPY_AND_ASSIGN(RuntimeContext);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_CONTEXT_H_
