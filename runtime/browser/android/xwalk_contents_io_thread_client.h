// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_IO_THREAD_CLIENT_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_IO_THREAD_CLIENT_H_

#include <string>

#include "base/memory/scoped_ptr.h"

class GURL;

namespace net {
class URLRequest;
}

namespace xwalk {

class InterceptedRequestData;

// This class provides a means of calling Java methods on an instance that has
// a 1:1 relationship with a WebContents instance directly from the IO thread.
//
// Specifically this is used to associate URLRequests with the WebContents that
// the URLRequest is made for.
//
// The native class is intended to be a short-lived handle that pins the
// Java-side instance. It is preferable to use the static getter methods to
// obtain a new instance of the class rather than holding on to one for
// prolonged periods of time (see note for more details).
//
// Note: The native XWalkContentsIoThreadClient instance has a Global ref to
// the Java object. By keeping the native XWalkContentsIoThreadClient
// instance alive you're also prolonging the lifetime of the Java instance, so
// don't keep a XWalkContentsIoThreadClient if you don't need to.
class XWalkContentsIoThreadClient {
 public:
  // Corresponds to WebSettings cache mode constants.
  enum CacheMode {
    LOAD_DEFAULT = -1,
    LOAD_NORMAL = 0,
    LOAD_CACHE_ELSE_NETWORK = 1,
    LOAD_NO_CACHE = 2,
    LOAD_CACHE_ONLY = 3,
  };

  virtual ~XWalkContentsIoThreadClient() {}

  // Returns whether this is a new pop up that is still waiting for association
  // with the java counter part.
  virtual bool PendingAssociation() const = 0;

  // Retrieve CacheMode setting value of this XWalkContent.
  // This method is called on the IO thread only.
  virtual CacheMode GetCacheMode() const = 0;

  // This will attempt to fetch the XWalkContentsIoThreadClient for the given
  // |render_process_id|, |render_frame_id| pair.
  // This method can be called from any thread.
  // An empty scoped_ptr is a valid return value.
  static scoped_ptr<XWalkContentsIoThreadClient> FromID(int render_process_id,
                                                        int render_frame_id);

  // Called on the IO thread when a subframe is created.
  static void SubFrameCreated(int render_process_id,
                              int parent_render_frame_id,
                              int child_render_frame_id);

  // This method is called on the IO thread only.
  virtual scoped_ptr<InterceptedRequestData> ShouldInterceptRequest(
      const GURL& location,
      const net::URLRequest* request) = 0;

  // Retrieve the AllowContentAccess setting value of this XWalkContent.
  // This method is called on the IO thread only.
  virtual bool ShouldBlockContentUrls() const = 0;

  // Retrieve the AllowFileAccess setting value of this XWalkContent.
  // This method is called on the IO thread only.
  virtual bool ShouldBlockFileUrls() const = 0;

  // Retrieve the BlockNetworkLoads setting value of this XWalkContent.
  // This method is called on the IO thread only.
  virtual bool ShouldBlockNetworkLoads() const = 0;

  // Called when ResourceDispathcerHost detects a download request.
  // The download is already cancelled when this is called, since
  // relevant for DownloadListener is already extracted.
  virtual void NewDownload(const GURL& url,
                           const std::string& user_agent,
                           const std::string& content_disposition,
                           const std::string& mime_type,
                           int64 content_length) = 0;

  // Called when a new login request is detected. See the documentation for
  // WebViewClient.onReceivedLoginRequest for arguments. Note that |account|
  // may be empty.
  virtual void NewLoginRequest(const std::string& realm,
                               const std::string& account,
                               const std::string& args) = 0;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_IO_THREAD_CLIENT_H_
