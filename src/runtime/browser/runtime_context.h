// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_RUNTIME_CONTEXT_H_
#define CAMEO_SRC_RUNTIME_BROWSER_RUNTIME_CONTEXT_H_

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"

namespace net {
class URLRequestContextGetter;
}

namespace content {
class DownloadManagerDelegate;
}

namespace cameo {

class RuntimeURLRequestContextGetter;

class RuntimeContext : public content::BrowserContext {
 public:
  RuntimeContext();
  virtual ~RuntimeContext();

  // BrowserContext implementation.
  virtual base::FilePath GetPath() OVERRIDE;
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
  virtual content::SpeechRecognitionPreferences*
      GetSpeechRecognitionPreferences() OVERRIDE;
  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;

  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers);
  net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers);

 private:
  class RuntimeResourceContext;

  // Performs initialization of the RuntimeContext while IO is still
  // allowed on the current thread.
  void InitWhileIOAllowed();

  scoped_ptr<RuntimeResourceContext> resource_context_;
  scoped_refptr<RuntimeURLRequestContextGetter> url_request_getter_;

  DISALLOW_COPY_AND_ASSIGN(RuntimeContext);
};

}  // namespace cameo

#endif  // CAMEO_SRC_RUNTIME_BROWSER_RUNTIME_CONTEXT_H_
