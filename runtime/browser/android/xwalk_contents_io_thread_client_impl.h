// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_IO_THREAD_CLIENT_IMPL_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_IO_THREAD_CLIENT_IMPL_H_

#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client.h"

#include <memory>
#include <string>

#include "base/android/scoped_java_ref.h"
#include "base/compiler_specific.h"

class GURL;

namespace content {
class WebContents;
}

namespace net {
class URLRequest;
}

namespace xwalk {

class XWalkWebResourceResponse;

class XWalkContentsIoThreadClientImpl : public XWalkContentsIoThreadClient {
 public:
  // Called when XWalkContent is created before there is a Java client.
  static void RegisterPendingContents(content::WebContents* web_contents);

  // Associates the |jclient| instance (which must implement the
  // XWalkContentsIoThreadClient Java interface) with the |web_contents|.
  // This should be called at most once per |web_contents|.
  static void Associate(content::WebContents* web_contents,
                        const base::android::JavaRef<jobject>& jclient);

  // Either |pending_associate| is true or |jclient| holds a non-null
  // Java object.
  XWalkContentsIoThreadClientImpl(
      bool pending_associate,
      const base::android::JavaRef<jobject>& jclient);
  ~XWalkContentsIoThreadClientImpl() override;

  // Implementation of XWalkContentsIoThreadClient.
  bool PendingAssociation() const override;
  CacheMode GetCacheMode() const override;
  std::unique_ptr<XWalkWebResourceResponse> ShouldInterceptRequest(
      const GURL& location,
      const net::URLRequest* request) override;
  bool ShouldBlockContentUrls() const override;
  bool ShouldBlockFileUrls() const override;
  bool ShouldBlockNetworkLoads() const override;
  void NewDownload(const GURL& url,
                   const std::string& user_agent,
                   const std::string& content_disposition,
                   const std::string& mime_type,
                   int64_t content_length) override;
  void NewLoginRequest(const std::string& realm,
                       const std::string& account,
                       const std::string& args) override;

  void OnReceivedResponseHeaders(
    const net::URLRequest* request,
    const net::HttpResponseHeaders* response_headers) override;

 private:
  bool pending_association_;
  base::android::ScopedJavaGlobalRef<jobject> java_object_;

  DISALLOW_COPY_AND_ASSIGN(XWalkContentsIoThreadClientImpl);
};

// JNI registration method.
bool RegisterXWalkContentsIoThreadClientImpl(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_IO_THREAD_CLIENT_IMPL_H_
